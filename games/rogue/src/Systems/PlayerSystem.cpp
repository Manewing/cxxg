#include "Systems/PlayerSystem.h"
#include "Components/AI.h"
#include "Components/Player.h"
#include "Components/Stats.h"
#include "Components/Transform.h"
#include "Components/Visual.h"
#include "Level.h"
#include <entt/entt.hpp>

#include "History.h"

PlayerSystem::PlayerSystem(Level &L) : System(L.Reg), L(L) {}

void PlayerSystem::update() {
  auto View =
      Reg.view<PlayerComp, PositionComp, const MeleeAttackComp, MovementComp>();
  View.each([this](const auto &Entity, auto &PC, auto &PosComp, const auto &MA,
                   auto &MC) {
    if (MC.Dir == ymir::Dir2d::NONE) {
      return;
    }
    PC.CurrentInteraction = std::nullopt;

    auto NewPos = PosComp.Pos + MC.Dir;
    MC.Dir = ymir::Dir2d::NONE;

    if (auto Et = L.getEntityAt(NewPos);
        Et != entt::null && Reg.all_of<HealthComp, FactionComp>(Et)) {

      // FIXME create damage system and spawn melee damage entity
      auto &THealth = Reg.get<HealthComp>(Et);

      int NewHealth = THealth.Health - MA.Damage;
      if (NewHealth < 0) {
        NewHealth = 0;
      }

      // publish
      const auto Nm = Reg.try_get<NameComp>(Entity);
      const auto TNm = Reg.try_get<NameComp>(Et);
      if (Nm && TNm) {
        publish(DebugMessageEvent() << Nm->Name << " dealt " << MA.Damage
                                    << " melee damage to " << TNm->Name);
      }

      THealth.Health = NewHealth;
    }

    if (!L.isBodyBlocked(NewPos)) {
      PosComp.Pos = NewPos;
    } else {
      publish(DebugMessageEvent() << "Can't move");
    }
  });
}