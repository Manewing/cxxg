#ifndef ROGUE_ENTITY_H
#define ROGUE_ENTITY_H

#include "Tile.h"
#include <ymir/Types.hpp>
#include <optional>
#include <functional>

class Level;

class Entity {
public:
  ymir::Point2d<int> Pos;
  Tile T;

  Entity(ymir::Point2d<int> Pos, Tile T) : Pos(Pos), T(T) {}
  virtual ~Entity() = default;

  virtual void update(Level*) {}

  bool isAlive() const { return Health != 0; }

  unsigned Health = 100;
  unsigned Agility = 100;
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
  void update(Level* L) override;

private:
  void wander(Level *L);

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