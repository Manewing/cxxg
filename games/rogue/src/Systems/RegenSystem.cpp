#include "Systems/RegenSystem.h"
#include "Components/Stats.h"
#include <entt/entt.hpp>

static constexpr float HealthPerVit = 9.1f;
static constexpr float ManaPerInt = 9.1f;

namespace {

template <typename Component>
void runRegenUpdate(entt::registry &Reg) {
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

} // namespace

void RegenSystem::update() {
  auto StatsView = Reg.view<const StatsComp, HealthComp, ManaComp>();
  StatsView.each([](const auto &St, auto &Health, auto &Mana) {
    Health.MaxValue = St.Vit * HealthPerVit;
    Mana.MaxValue = St.Int * ManaPerInt;
  });
  runRegenUpdate<HealthComp>(Reg);
  runRegenUpdate<ManaComp>(Reg);
}