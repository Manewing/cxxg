#include <entt/entt.hpp>
#include <rogue/Components/AI.h>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Event.h>
#include <rogue/Level.h>
#include <rogue/Systems/PlayerSystem.h>

namespace rogue {

void updatePlayerPosition(Level &L, entt::registry &Reg, EventHubConnector &EHC,
                          entt::entity PlayerEt, PlayerComp &PC,
                          PositionComp &PosComp, AgilityComp &AG) {
  if (PC.MoveDir == ymir::Dir2d::NONE) {
    return;
  }
  PC.CurrentInteraction = std::nullopt;

  auto NewPos = PosComp.Pos + PC.MoveDir;
  const auto MoveDir = PC.MoveDir;

  // FIXME this does not count in combat
  // -> check instead of an action was successfully performed
  PC.IsReady = AG.hasEnoughAP(MovementComp::MoveAPCost);

  if (!PC.IsReady) {
    return;
  }

  if (auto Et = L.getEntityAt(NewPos);
      Et != entt::null && Reg.all_of<HealthComp, FactionComp>(Et)) {
    Reg.emplace<CombatComp>(PlayerEt, Et);
    return;
  }

  if (L.isBodyBlocked(NewPos)) {
    EHC.publish(PlayerInfoMessageEvent() << "Can't move");
  }

  // Always move the player, even if blocked to consume AP for action
  MovementComp MC;
  MC.Dir = MoveDir;
  Reg.emplace<MovementComp>(PlayerEt, MC);
}

PlayerSystem::PlayerSystem(Level &L) : System(L.Reg), L(L) {}

void PlayerSystem::update(UpdateType Type) {
  if (Type == UpdateType::NoTick) {
    return;
  }

  auto View = Reg.view<PlayerComp, PositionComp, AgilityComp>();
  View.each([this](const auto &PlayerEt, auto &PC, auto &PosComp, auto &AG) {
    updatePlayerPosition(L, Reg, *this, PlayerEt, PC, PosComp, AG);
  });
}

} // namespace rogue