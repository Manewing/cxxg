#ifndef ROGUE_COMPONENTS_AI_H
#define ROGUE_COMPONENTS_AI_H

#include <rogue/Tile.h>

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

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_AI_H