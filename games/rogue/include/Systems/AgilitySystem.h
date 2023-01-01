#ifndef ROGUE_SYSTEMS_AGILITY_SYSTEM_H
#define ROGUE_SYSTEMS_AGILITY_SYSTEM_H

#include <entt/entt.hpp>

#include "Components/Stats.h"

class AgilitySystem {
public:
  explicit AgilitySystem(entt::registry &Reg) : Reg(Reg) {}

  void update() {
    auto View = Reg.view<AgilityComp>();
    View.each([](auto &Ag) {
      constexpr float APPerAgilityPoint = 0.12f;
      Ag.gainAP(static_cast<unsigned>(APPerAgilityPoint * Ag.Agility));
    });
  }

private:
  entt::registry &Reg;
};

#endif // #ifndef ROGUE_SYSTEMS_AGILITY_SYSTEM_H