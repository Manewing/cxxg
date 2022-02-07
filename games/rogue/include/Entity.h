#ifndef ROGUE_ENTITY_H
#define ROGUE_ENTITY_H

#include "Tile.h"
#include <ymir/Types.hpp>

class Entity {
public:
  ymir::Point2d<int> Pos;
  Tile T;

  Entity(ymir::Point2d<int> Pos, Tile T) : Pos(Pos), T(T) {}

  bool isAlive() const { return Health != 0; }

  unsigned Health = 100;
  unsigned Agility = 100;
};

// FIXME move
class Enemy : public Entity {
public:
  using Entity::Entity;
};

#endif // #ifndef ROGUE_ENTITY_H