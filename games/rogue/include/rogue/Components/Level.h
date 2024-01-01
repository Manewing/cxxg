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
  bool IsOpen = false;
  Tile OpenTile;
  Tile ClosedTile;
  std::optional<int> KeyId;

  bool hasLock() const { return KeyId.has_value(); }
  bool isLocked() const { return hasLock() && !IsOpen; }
};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_LEVEL_H