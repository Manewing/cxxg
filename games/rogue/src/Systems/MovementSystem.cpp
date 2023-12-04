#include <rogue/Components/Transform.h>
#include <rogue/Level.h>
#include <rogue/Systems/MovementSystem.h>

namespace rogue {

namespace {

void applyVectorMovementComp(entt::registry &Reg, Level &L, entt::entity Entity,
                             PositionComp &PC, VectorMovementComp &VMC) {
  auto *AG = Reg.try_get<AgilityComp>(Entity);
  if (AG && !AG->trySpendAP(MovementComp::MoveAPCost)) {
    return;
  }

  // Compute new position
  auto NewPos = VMC.Vector.normalized() + VMC.LastPos;
  if (VMC.Flying || !L.isBodyBlocked(NewPos.to<int>())) {
    L.updateEntityPosition(Entity, PC, NewPos.to<int>());
    VMC.LastPos = NewPos;
  }

  if (VMC.KillOnWall && L.isWallBlocked(NewPos.to<int>())) {
    Reg.destroy(Entity);
  }
}

void applyMovementComp(entt::registry &Reg, Level &L, entt::entity Entity,
                       PositionComp &PC, const MovementComp &MC) {
  auto *AG = Reg.try_get<AgilityComp>(Entity);
  if (AG && !AG->trySpendAP(MovementComp::MoveAPCost)) {
    Reg.erase<MovementComp>(Entity);
    return;
  }

  if (MC.Dir == ymir::Dir2d::NONE) {
    AG->trySpendAP(MovementComp::MoveAPCost);
    Reg.erase<MovementComp>(Entity);
    return;
  }

  auto NewPos = PC.Pos + MC.Dir;
  if (MC.Flying || !L.isBodyBlocked(NewPos)) {
    L.updateEntityPosition(Entity, PC, NewPos);
  }

  if (MC.KillOnWall && L.isWallBlocked(NewPos)) {
    Reg.destroy(Entity);
  } else {
    Reg.erase<MovementComp>(Entity);
  }
}

} // namespace

MovementSystem::MovementSystem(Level &L) : System(L.Reg), L(L) {}

void MovementSystem::update(UpdateType Type) {
  if (Type == UpdateType::NoTick) {
    return;
  }

  // Deal with position seeking
  Reg.view<PositionComp, VectorMovementComp>().each(
      [this](const auto &Et, auto &PC, auto &VMC) {
        applyVectorMovementComp(Reg, L, Et, PC, VMC);
      });

  Reg.view<PositionComp, MovementComp>().each(
      [this](auto Entity, auto &PC, auto &MC) {
        applyMovementComp(Reg, L, Entity, PC, MC);
      });
}

} // namespace rogue