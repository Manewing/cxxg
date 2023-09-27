#include <entt/entt.hpp>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Stats.h>
#include <rogue/Event.h>
#include <rogue/Systems/RegenSystem.h>

namespace rogue {

namespace {

template <typename T>
using IsRegenComp = std::is_base_of<ValueRegenCompBase, T>;

template <typename T>
using IsRegenerationBuff = std::is_base_of<RegenerationBuff, T>;

template <typename T> using IsReductionBuff = std::is_base_of<ReductionBuff, T>;

template <typename Component>
typename std::enable_if_t<IsRegenComp<Component>::value>
runRegenUpdate(entt::registry &Reg) {
  auto View = Reg.view<Component>();
  View.each([](auto &C) {
    // Check there are ticks left until the next update, pre-decrement so
    // tick period of 1 means every tick
    if (--C.TicksLeft != 0) {
      return;
    }
    C.TicksLeft = C.TickPeriod;

    // Restore the given amount to regenerate
    C.restore(C.RegenAmount);
  });
}

template <typename Component, typename Buff>
typename std::enable_if_t<IsRegenComp<Component>::value &&
                          IsRegenerationBuff<Buff>::value>
runRegenBuffUpdate(System &Sys, entt::registry &Reg) {
  auto View = Reg.view<Buff, Component>();
  View.each([&Sys, &Reg](auto Entity, auto &B, auto &C) {
    // Post decrement to match count
    if (B.TicksLeft-- == 0) {
      BuffExpiredEvent BEE;
      BEE.Entity = Entity;
      BEE.Buff = &B;
      Sys.publish(BEE);
      Reg.erase<Buff>(Entity);
      return;
    }
    // Restore the given amount to regenerate
    C.restore(B.RegenAmount);
  });
}

template <typename Component, typename Buff>
typename std::enable_if_t<IsRegenComp<Component>::value &&
                          IsReductionBuff<Buff>::value>
runReductionBuffUpdate(System &Sys, entt::registry &Reg) {
  auto View = Reg.view<Buff, Component>();
  View.each([&Sys, &Reg](auto Entity, auto &B, auto &C) {
    // Post decrement to match count
    if (B.TicksLeft-- == 0) {
      Sys.publish(DebugMessageEvent() << B.getName() << " expired");
      Reg.erase<Buff>(Entity);
      return;
    }
    // Reduce the given amount
    C.reduce(B.ReduceAmount);
  });
}

} // namespace

void RegenSystem::update(UpdateType Type) {
  if (Type != UpdateType::Tick) {
    return;
  }

  // Run regeneration
  runRegenUpdate<HealthComp>(Reg);
  runRegenUpdate<ManaComp>(Reg);

  // Process buffs
  runRegenBuffUpdate<HealthComp, HealthRegenBuffComp>(*this, Reg);
  runRegenBuffUpdate<ManaComp, ManaRegenBuffComp>(*this, Reg);
  runReductionBuffUpdate<HealthComp, PoisonDebuffComp>(*this, Reg);
  runReductionBuffUpdate<HealthComp, BleedingDebuffComp>(*this, Reg);
}

} // namespace rogue