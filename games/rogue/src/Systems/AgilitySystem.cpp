#include <entt/entt.hpp>
#include <rogue/Components/Stats.h>
#include <rogue/Systems/AgilitySystem.h>

static constexpr float APPerAgilityPoint = 2.0f;

namespace rogue {

void AgilitySystem::update(UpdateType Type) {
  if (Type == UpdateType::NoTick) {
    return;
  }
  auto View = Reg.view<AgilityComp>();
  View.each([](auto &Ag) {
    Ag.gainAP(static_cast<unsigned>(APPerAgilityPoint * Ag.Agility));
  });
}

} // namespace rogue