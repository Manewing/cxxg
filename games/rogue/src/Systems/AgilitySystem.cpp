#include "Systems/AgilitySystem.h"
#include "Components/Stats.h"
#include <entt/entt.hpp>

void AgilitySystem::update() {
  auto View = Reg.view<AgilityComp>();
  View.each([](auto &Ag) {
    constexpr float APPerAgilityPoint = 0.12f;
    Ag.gainAP(static_cast<unsigned>(APPerAgilityPoint * Ag.Agility));
  });
}