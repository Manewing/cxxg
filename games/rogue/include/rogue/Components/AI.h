#ifndef ROGUE_COMPONENTS_AI_H
#define ROGUE_COMPONENTS_AI_H

#include <rogue/Tile.h>
#include <iosfwd>

namespace rogue {

enum class WanderAIState { Idle = 0, Wander = 1, Chase = 2 };

struct WanderAIComp {
  WanderAIState State = WanderAIState::Idle;
  unsigned IdleDelay = 2;
  unsigned IdleDelayLeft = 2;
};

struct LineOfSightComp {
  unsigned LOSRange = 14;
};

struct AttackAIComp {};

enum class NeedKind {
  // Model needs like Maslow
  NONE,
  DRINK,
  FOOD,
  SLEEP,
  SHELTER,
  CLOTHING
};
const char *getNeedKindStr(NeedKind Need);
std::ostream &operator<<(std::ostream &Out, NeedKind Need);

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

ActionState getActionFromNeed(NeedKind Need);

struct PhysState {
  unsigned Thirst = 1000;
  unsigned Hunger = 1000;
  unsigned Fatigue = 1000;

  NeedKind getBiggestNeed() const;
  void update();
};
std::ostream &operator<<(std::ostream &Out, const PhysState &PS);

struct ReasoningStateComp {
  ActionState State = ActionState::IDLE;
  int UpdateCooldown = 5;
};




} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_AI_H