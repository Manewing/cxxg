#ifndef ROGUE_COMPONENTS_AI_H
#define ROGUE_COMPONENTS_AI_H

#include <rogue/Tile.h>

namespace rogue {

enum class WanderAIState { Idle = 0, Wander = 1, Chase = 2};

struct WanderAIComp {
  WanderAIState State = WanderAIState::Idle;
  unsigned IdleDelay = 2;
  unsigned IdleDelayLeft = 2;
};

struct LineOfSightComp {
  unsigned LOSRange = 14;
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