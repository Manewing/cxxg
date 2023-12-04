#include <rogue/Components/RaceFaction.h>

namespace rogue {

FactionKind getFaction(std::string_view Name) {
  if (Name == "nature") {
    return FactionKind::Nature;
  }
  if (Name == "enemy") {
    return FactionKind::Enemy;
  }
  if (Name == "player") {
    return FactionKind::Player;
  }
  return FactionKind::None;
}

RaceKind getRace(std::string_view Name) {
  if (Name == "human") {
    return RaceKind::Human;
  }
  if (Name == "elf") {
    return RaceKind::Elf;
  }
  if (Name == "dwarf") {
    return RaceKind::Dwarf;
  }
  if (Name == "orc") {
    return RaceKind::Orc;
  }
  if (Name == "troll") {
    return RaceKind::Troll;
  }
  if (Name == "goblin") {
    return RaceKind::Goblin;
  }
  if (Name == "undead") {
    return RaceKind::Undead;
  }
  if (Name == "creature") {
    return RaceKind::Creature;
  }
  if (Name == "dummy") {
    return RaceKind::Dummy;
  }
  return RaceKind::None;
}

} // namespace rogue