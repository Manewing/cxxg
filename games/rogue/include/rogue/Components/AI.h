#ifndef ROGUE_COMPONENTS_AI_H
#define ROGUE_COMPONENTS_AI_H

#include <rogue/Tile.h>

namespace rogue {

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

enum class RaceKind { Human, Elf, Dwarf, Orc, Troll, Goblin, Undead, Creature };

struct RaceComp {
  RaceKind Kind;
};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_AI_H