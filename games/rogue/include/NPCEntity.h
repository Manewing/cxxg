#ifndef ROGUE_NPC_ENTITY_H
#define ROGUE_NPC_ENTITY_H

#include "Entity.h"

struct PhysState {
  unsigned Thirst = 1000;
  unsigned Hunger = 1000;
  unsigned Fatigue = 1000;
};
std::ostream &operator<<(std::ostream &Out, const PhysState &PS);

enum class ActionState {
  //
  IDLE,
  WANDER,
  SEARCH_DRINK,
  SEARCH_FOOD,
  SLEEP
};
const char *getActionStateStr(ActionState AS);
std::ostream &operator<<(std::ostream &Out, ActionState AS);

enum class NeedKind {
  //
  NONE,
  DRINK,
  FOOD,
  SLEEP
};
const char *getNeedKindStr(NeedKind Need);
std::ostream &operator<<(std::ostream &Out, NeedKind Need);

class NPCEntity : public Entity {
public:
  using Entity::Entity;
  void update(Level &L) override;

  // protected:
  void updatePhysState();
  void decideAction();

  NeedKind getBiggestNeed() const;
  static ActionState getActionFromNeed(NeedKind Need);

  void handleAction(Level &L);

  void searchObject(Level &L, Tile T,
                    std::function<void(ymir::Point2d<int>)> FoundCallback);

  // protected:
  // FIXME hack simulating time passing while searching
  int SearchCooldown = 5;
  PhysState PS;
  ActionState CurrentActionState = ActionState::IDLE;
};

#endif // #ifndef ROGUE_NPC_ENTITY_H