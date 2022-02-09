#include "Entity.h"
#include "Level.h"
#include <random>
#include <ymir/Noise.hpp>

// FIXME move this, also should this be based on the level seed?
static std::random_device RandomEngine;

void EnemyEntity::update(Level *L) {
  switch (CurrentState) {
  case State::Idle:
    CurrentState = State::Wander;
    break;
  case State::Wander:
    wander(L);
    CurrentState = State::Idle;
    break;
  case State::Chase:
    break;
  }
}

void EnemyEntity::wander(Level *L) {
  auto AllNextPos = L->getAllNonBodyBlockedPosNextTo(Pos);
  auto It =
      ymir::randomIterator(AllNextPos.begin(), AllNextPos.end(), RandomEngine);
  if (It == AllNextPos.end()) {
    return;
  }
  Pos = *It;
}

PlayerEntity::PlayerEntity(ymir::Point2d<int> Pos) : Entity(Pos, PlayerTile) {}