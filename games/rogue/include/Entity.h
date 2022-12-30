#ifndef ROGUE_ENTITY_H
#define ROGUE_ENTITY_H

#include "EventHub.h"
#include "Inventory.h"
#include "Tile.h"
#include <exception>
#include <functional>
#include <optional>
#include <ymir/Types.hpp>

class Level;
class Entity;

class MovementBlockedException : public std::exception {
public:
  MovementBlockedException(ymir::Point2d<int> Pos);
  const char *what() const noexcept final;

public:
  const ymir::Point2d<int> Pos;

private:
  std::string Msg;
};

struct EntityAttackEvent : public Event {
  Entity &Attacker;
  Entity &Target;
  unsigned Damage;

  bool isPlayerAffected() const;
};

struct EntityDiedEvent : public Event {
  Entity &Et;
  bool isPlayer() const;
};

class Entity : public EventHubConnector {
public:
  ymir::Point2d<int> Pos;
  Tile T;

  Entity(ymir::Point2d<int> Pos, Tile T, std::string Name = "TODO_Skeleton")
      : Pos(Pos), T(T), Inv(*this), Name(Name) {}
  virtual ~Entity() = default;

  inline const std::string &getName() const { return Name; }

  virtual void update(Level &) {}

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

private:
  std::string Name;
};

// FIXME move
class EnemyEntity : public Entity {
public:
  enum class State { Idle, Wander, Chase };

public:
  using Entity::Entity;
  void update(Level &L) override;

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
    std::function<void()> Finalize = []() {};
  };

public:
  PlayerEntity(ymir::Point2d<int> Pos = {0, 0});
  std::optional<Interaction> CurrentInteraction;
};

#endif // #ifndef ROGUE_ENTITY_H