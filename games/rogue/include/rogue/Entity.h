#ifndef ROGUE_ENTITY_H
#define ROGUE_ENTITY_H

#include <exception>
#include <functional>
#include <optional>
#include <rogue/EventHub.h>
#include <rogue/Inventory.h>
#include <rogue/Tile.h>
#include <ymir/Types.hpp>

namespace rogue {
class Level;
class Entity;
} // namespace rogue

namespace rogue {

class MovementBlockedException : public std::exception {
public:
  MovementBlockedException(ymir::Point2d<int> Pos);
  const char *what() const noexcept final;

public:
  const ymir::Point2d<int> Pos;

private:
  std::string Msg;
};


class Entity : public EventHubConnector {
public:
  ymir::Point2d<int> Pos;
  Tile T;

  Entity(ymir::Point2d<int> Pos, Tile T, std::string Name = "TODO_Skeleton")
      : Pos(Pos), T(T), Name(Name) {}
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
};

} // namespace rogue

#endif // #ifndef ROGUE_ENTITY_H