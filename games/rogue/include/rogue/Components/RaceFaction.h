#ifndef ROGUE_COMPONENTS_RACE_FACTION_H
#define ROGUE_COMPONENTS_RACE_FACTION_H

#include <string_view>

namespace rogue {

enum class FactionKind { None = 0x0, Nature = 0x1, Enemy = 0x2, Player = 0x4 };

FactionKind getFaction(std::string_view Name);

struct FactionComp {
  FactionKind Faction = FactionKind::Nature;
};

enum class RaceKind {
  None = 0x0,
  Human = 0x1,
  Elf = 0x2,
  Dwarf = 0x4,
  Orc = 0x8,
  Troll = 0x10,
  Goblin = 0x20,
  Undead = 0x40,
  Creature = 0x80
};

RaceKind getRace(std::string_view Name);

struct RaceComp {
  RaceKind Kind;
};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_RACE_FACTION_H