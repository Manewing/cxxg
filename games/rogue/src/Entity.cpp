#include "Entity.h"

void EnemyEntity::update() {
  switch (CurrentState) {
  case State::Idle:
    break;
  case State::Wander:
    break;
  case State::Chase:
    break;
  }
}

PlayerEntity::PlayerEntity(ymir::Point2d<int> Pos) : Entity(Pos, PlayerTile) {}