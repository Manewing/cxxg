#include <entt/entt.hpp>
#include <rogue/Components/Stats.h>
#include <rogue/Systems/AgilitySystem.h>

static constexpr float APPerAgilityPoint = 0.1f;

namespace rogue {

namespace {
void updateAP(AgilityComp &Ag) { Ag.gainAP(APPerAgilityPoint * Ag.Agility); }
} // namespace

void AgilitySystem::update(UpdateType Type) {
  if (Type == UpdateType::NoTick) {
    return;
  }
  auto View = Reg.view<AgilityComp>();
  View.each([](auto &Ag) { updateAP(Ag); });
}

} // namespace rogue