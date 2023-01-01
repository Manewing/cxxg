#ifndef ROGUE_COMPONENTS_AI_H
#define ROGUE_COMPONENTS_AI_H

#include "Tile.h"

enum class WanderAIState { Idle, Wander, Chase };

struct WanderAIComp {
  WanderAIState State = WanderAIState::Idle;
  unsigned IdleDelay = 2;
  unsigned IdleDelayLeft = 2;
  unsigned MoveAPCost = 5;
  unsigned DistanceWalked = 0;
};

struct LineOfSightComp {
  unsigned LOSRange = 8;
};

struct AttackAIComp {};

enum class FactionKind { Nature, Enemy, Player };

struct FactionComp {
  FactionKind Faction = FactionKind::Nature;
};

#endif // #ifndef ROGUE_COMPONENTS_AI_H