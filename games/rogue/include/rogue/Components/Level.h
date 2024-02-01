#ifndef ROGUE_COMPONENTS_LEVEL_H
#define ROGUE_COMPONENTS_LEVEL_H

#include <optional>
#include <rogue/Components/Visual.h>

namespace rogue {

struct LevelStartComp {
  static std::string getName() { return "LevelStart"; }
  int NextLevelId = -1;
};

struct LevelEndComp {
  static std::string getName() { return "LevelEnd"; }
  int NextLevelId = -1;
};

struct DoorComp {
  static bool unlockDoor(entt::registry &Reg, const entt::entity &DoorEt,
                         const entt::entity &ActEt);
  static void openDoor(entt::registry &Reg, const entt::entity &Entity);
  static void closeDoor(entt::registry &Reg, const entt::entity &Entity);

  bool IsOpen = false;
  Tile OpenTile;
  Tile ClosedTile;
  std::optional<int> KeyId;
  std::size_t ActionIdx = -1UL;

  bool hasLock() const { return KeyId.has_value(); }
  bool isLocked() const { return hasLock() && !IsOpen; }

  template <class Archive> void serialize(Archive &Ar) {
    Ar(IsOpen, OpenTile, ClosedTile, KeyId, ActionIdx);
  }
};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_LEVEL_H