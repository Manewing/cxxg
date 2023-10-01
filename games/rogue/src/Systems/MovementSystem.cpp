#include <rogue/Components/Transform.h>
#include <rogue/Level.h>
#include <rogue/Systems/MovementSystem.h>

namespace rogue {

namespace {

void applyMovementComp(entt::registry &Reg, Level &L, entt::entity Entity,
                       PositionComp &PC, const MovementComp &MC) {
  if (MC.Dir == ymir::Dir2d::NONE) {
    if (MC.Clear) {
      Reg.erase<MovementComp>(Entity);
    }
    return;
  }

  auto *AG = Reg.try_get<AgilityComp>(Entity);
  if (AG && !AG->trySpendAP(MovementComp::MoveAPCost)) {
    if (MC.Clear) {
      Reg.erase<MovementComp>(Entity);
    }
    return;
  }

  auto NewPos = PC.Pos + MC.Dir;
  if (MC.Flying || !L.isBodyBlocked(NewPos)) {
    L.updateEntityPosition(Entity, PC, NewPos);
  }

  if (MC.KillOnWall && L.isWallBlocked(NewPos)) {
    Reg.destroy(Entity);
  } else if (MC.Clear) {
    Reg.erase<MovementComp>(Entity);
  }
}

} // namespace

MovementSystem::MovementSystem(Level &L) : System(L.Reg), L(L) {}

void MovementSystem::update(UpdateType Type) {
  if (Type == UpdateType::NoTick) {
    return;
  }

  auto View = Reg.view<PositionComp, MovementComp>();
  View.each([this](auto Entity, auto &PC, auto &MC) {
    applyMovementComp(Reg, L, Entity, PC, MC);
  });
}

} // namespace rogue