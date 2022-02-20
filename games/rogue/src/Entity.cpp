#include "Entity.h"
#include "Level.h"
#include <random>
#include <sstream>
#include <ymir/Algorithm/Dijkstra.hpp>
#include <ymir/Algorithm/LineOfSight.hpp>
#include <ymir/Noise.hpp>

#include <ymir/Algorithm/DijkstraIo.hpp>

// FIXME move this, also should this be based on the level seed?
static std::random_device RandomEngine;

MovementBlockedException::MovementBlockedException(ymir::Point2d<int> Pos)
    : Pos(Pos) {
  std::stringstream SS;
  SS << "Movement blocked at " << Pos;
  Msg = SS.str();
}

const char *MovementBlockedException::what() const noexcept {
  return Msg.c_str();
}

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
}

void Entity::wander(Level &L) {
  auto AllNextPos = L.getAllNonBodyBlockedPosNextTo(Pos);
  auto It =
      ymir::randomIterator(AllNextPos.begin(), AllNextPos.end(), RandomEngine);
  if (It == AllNextPos.end()) {
    throw MovementBlockedException(Pos);
  }
  Pos = *It;
}

void Entity::heal(unsigned Amount) {
  Health = std::min(Health + Amount, MaxHealth);
}

void Entity::damage(unsigned Amount) {
  int NewHealth = Health - Amount;
  if (NewHealth < 0) {
    NewHealth = 0;
  }
  Health = NewHealth;
}

void EnemyEntity::update(Level &L) {
  switch (CurrentState) {
  case State::Idle:
    // FIXME switch after random duration
    CurrentState = State::Wander;
    if (checkForPlayer(L)) {
      CurrentState = State::Chase;
    }
    break;
  case State::Wander:
    wander(L);
    // FIXME switch after random duration
    CurrentState = State::Idle;
    if (checkForPlayer(L)) {
      CurrentState = State::Chase;
    }
    break;
  case State::Chase:
    if (!checkForPlayer(L)) {
      CurrentState = State::Idle;
      break;
    }
    auto *Player = L.getPlayer();
    if (canAttack(Player->Pos)) {
      attackEntity(*Player);
    } else {
      chasePlayer(L);
    }
    break;
  }
}

bool EnemyEntity::checkForPlayer(Level &L) {
  auto *Player = L.getPlayer();
  if (!Player) {
    return false;
  }

  bool IsInLOS =
      ymir::Algorithm::isInLOS([&L](auto Pos) { return L.isLOSBlocked(Pos); },
                               Pos, Player->Pos, LOSRange);
  if (!IsInLOS) {
    return false;
  }
  return true;
}

void EnemyEntity::chasePlayer(Level &L) {
  auto PathToPlayer = ymir::Algorithm::getPathFromDijkstraMap(
      L.getPlayerDijkstraMap(), Pos, ymir::FourTileDirections<int>(), 1);
  auto TargetPos = PathToPlayer.at(0);

  if (!L.isBodyBlocked(TargetPos)) {
    Pos = PathToPlayer.at(0);
  }
}

PlayerEntity::PlayerEntity(ymir::Point2d<int> Pos) : Entity(Pos, PlayerTile) {
  Damage = 45;
}