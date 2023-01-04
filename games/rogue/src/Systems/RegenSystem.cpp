#include "Systems/RegenSystem.h"
#include "Components/Stats.h"
#include <entt/entt.hpp>

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

    // Update value by with regeneration amount, do not exceed max
    C.Value = std::min(C.Value + C.RegenAmount, C.MaxValue);
  });
}

} // namespace

void RegenSystem::update() {
  runRegenUpdate<HealthComp>(Reg);
  runRegenUpdate<ManaComp>(Reg);
}