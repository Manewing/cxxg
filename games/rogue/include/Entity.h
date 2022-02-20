#ifndef ROGUE_ENTITY_H
#define ROGUE_ENTITY_H

#include "Tile.h"
#include "Inventory.h"
#include <ymir/Types.hpp>
#include <optional>
#include <functional>
#include <exception>

class Level;

class MovementBlockedException : public std::exception {
public:
  MovementBlockedException(ymir::Point2d<int> Pos);
  const char *what() const noexcept final;

public:
  const ymir::Point2d<int> Pos;

private:
  std::string Msg;
};

class Entity {
public:
  ymir::Point2d<int> Pos;
  Tile T;

  Entity(ymir::Point2d<int> Pos, Tile T) : Pos(Pos), T(T), Inv(*this) {}
  virtual ~Entity() = default;

  virtual void update(Level&) {}

  bool isAlive() const { return Health != 0; }

  bool canAttack(ymir::Point2d<int> Pos) const;
  void attackEntity(Entity &Other);
  void wander(Level &L);

  void heal(unsigned Amount);
  void damage(unsigned Amount);

  unsigned Damage = 10;
  unsigned Health = 100;
  unsigned MaxHealth = 100;
  unsigned Agility = 100;
  unsigned LOSRange = 8;

  Inventory Inv;
};

// FIXME move
class EnemyEntity : public Entity {
public:
  enum class State {
    Idle,
    Wander,
    Chase
  };

public:
  using Entity::Entity;
  void update(Level& L) override;

private:
  bool checkForPlayer(Level &L);
  void chasePlayer(Level &L);

private:
  State CurrentState = State::Idle;
};

class PlayerEntity : public Entity {
public:
  static constexpr Tile PlayerTile{{'@', cxxg::types::RgbColor{255, 255, 50}}};
  struct Interaction {
    std::string Msg;
    std::function<void()> Finalize = [](){};
  };

public:
  PlayerEntity(ymir::Point2d<int> Pos = {0, 0});
  std::optional<Interaction> CurrentInteraction;
};

#endif // #ifndef ROGUE_ENTITY_H