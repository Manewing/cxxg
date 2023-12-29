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
using IsDimRetBuff = std::is_base_of<DiminishingReturnsValueGenBuff, T>;

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
                          IsDimRetBuff<Buff>::value>
runDimRetValueGenBuff(System &Sys, entt::registry &Reg, bool Reduce) {
  auto View = Reg.view<Buff, Component>();
  View.each([&Sys, &Reg, Reduce](auto Entity, auto &B, auto &C) {
    // Post decrement to match count
    auto St = B.tick();
    if (St == TimedBuff::State::Expired) {
      Sys.publish(BuffExpiredEvent{{}, Entity, &Reg, &B});
      Reg.erase<Buff>(Entity);
      return;
    }
    if (St == TimedBuff::State::Waiting) {
      return;
    }
    // Reduce the given amount
    Sys.publish(BuffApplyEffectEvent{{}, Entity, &Reg, Reduce, &B});
    if (Reduce) {
      C.reduce(B.TickAmount);
    } else {
      C.restore(B.TickAmount);
    }
  });
}

} // namespace

void RegenSystem::update(UpdateType Type) {
  if (Type != UpdateType::Tick) {
    return;
  }

  // Run mana regeneration
  runRegenUpdate<ManaComp>(Reg);
  runDimRetValueGenBuff<ManaComp, ManaRegenBuffComp>(*this, Reg,
                                                     /*Reduce=*/false);

  // Run health regeneration
  runRegenUpdate<HealthComp>(Reg);
  runDimRetValueGenBuff<HealthComp, HealthRegenBuffComp>(*this, Reg,
                                                         /*Reduce=*/false);

  // Run health reduction
  runDimRetValueGenBuff<HealthComp, PoisonDebuffComp>(*this, Reg,
                                                      /*Reduce=*/true);
  runDimRetValueGenBuff<HealthComp, BleedingDebuffComp>(*this, Reg,
                                                        /*Reduce=*/true);
}

} // namespace rogue