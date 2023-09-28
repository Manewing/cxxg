#include <rogue/Components/Transform.h>
#include <rogue/Level.h>
#include <rogue/Systems/MovementSystem.h>

namespace rogue {

namespace {

void applyMovementComp(entt::registry &Reg, Level &L, entt::entity Entity,
                       PositionComp &PC, const MovementComp &MC) {
  (void)Reg;
  (void)Entity;
  if (MC.Dir == ymir::Dir2d::NONE) {
    return;
  }
  auto NewPos = PC.Pos + MC.Dir;
  if (!L.isBodyBlocked(NewPos)) {
    L.updateEntityPosition(Entity, PC, NewPos);
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
    Reg.erase<MovementComp>(Entity);
  });
}

} // namespace rogue