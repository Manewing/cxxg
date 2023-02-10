#include <random>
#include <rogue/Entity.h>
#include <rogue/Level.h>
#include <sstream>

namespace rogue {

MovementBlockedException::MovementBlockedException(ymir::Point2d<int> Pos)
    : Pos(Pos) {
  std::stringstream SS;
  SS << "Movement blocked at " << Pos;
  Msg = SS.str();
}

const char *MovementBlockedException::what() const noexcept {
  return Msg.c_str();
}

bool EntityAttackEvent::isPlayerAffected() const { return false; }

bool Entity::canAttack(ymir::Point2d<int> Pos) const {
  // FIXME only melee check right now
  //   x
  //  xSx
  //   x
  auto Diff = (Pos - this->Pos).abs();
  return (Diff.X + Diff.Y) <= 1;
}

void Entity::attackEntity(Entity &Other) {
  Other.damage(Damage);
  publish(EntityAttackEvent{{}, *this, Other, Damage});
}

void Entity::wander(Level &L) { (void)L; }

void Entity::heal(unsigned Amount) {
  Health = std::min(Health + Amount, MaxHealth);
}

void Entity::damage(unsigned Amount) {
  int NewHealth = Health - Amount;
  if (NewHealth <= 0) {
    NewHealth = 0;
  }
  Health = NewHealth;
}

void EnemyEntity::update(Level &L) { (void)L; }

bool EnemyEntity::checkForPlayer(Level &L) {
  (void)L;
  return false;
}

void EnemyEntity::chasePlayer(Level &L) { (void)L; }

} // namespace rogue