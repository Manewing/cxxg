#include <entt/entt.hpp>
#include <rogue/Components/AI.h>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Level.h>
#include <rogue/Systems/PlayerSystem.h>

#include <rogue/History.h>

namespace rogue {

PlayerSystem::PlayerSystem(Level &L) : System(L.Reg), L(L) {}

void PlayerSystem::update(UpdateType Type) {
  if (Type == UpdateType::NoTick) {
    return;
  }

  auto View = Reg.view<PlayerComp, PositionComp, MovementComp>();
  View.each([this](const auto &PlayerEt, auto &PC, auto &PosComp, auto &MC) {
    if (MC.Dir == ymir::Dir2d::NONE) {
      return;
    }
    PC.CurrentInteraction = std::nullopt;

    auto NewPos = PosComp.Pos + MC.Dir;
    MC.Dir = ymir::Dir2d::NONE;

    if (auto Et = L.getEntityAt(NewPos);
        Et != entt::null && Reg.all_of<HealthComp, FactionComp>(Et)) {
      Reg.emplace<CombatComp>(PlayerEt, Et);
      return;
    }

    if (!L.isBodyBlocked(NewPos)) {
      L.updateEntityPosition(PlayerEt, PosComp, NewPos);
    } else {
      publish(DebugMessageEvent() << "Can't move");
    }
  });
}

} // namespace rogue