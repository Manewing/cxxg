#include "Systems/AgilitySystem.h"
#include "Components/Stats.h"
#include <entt/entt.hpp>

static constexpr float AgilityPerDex = 2.73f;
static constexpr float APPerAgilityPoint = 0.12f;

void AgilitySystem::update() {
  auto StatsView = Reg.view<const StatsComp, AgilityComp>();
  StatsView.each([](const auto &St, auto &Ag) {
    Ag.Agility = AgilityPerDex * St.Dex;
  });
  auto View = Reg.view<AgilityComp>();
  View.each([](auto &Ag) {
    constexpr float APPerAgilityPoint = 0.12f;
    Ag.gainAP(static_cast<unsigned>(APPerAgilityPoint * Ag.Agility));
  });
}