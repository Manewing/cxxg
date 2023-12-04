#ifndef ROGUE_COMPONENTS_LEVEL_H
#define ROGUE_COMPONENTS_LEVEL_H

#include <optional>

namespace rogue {

struct LevelStartComp {
  int NextLevelId = -1;
};

struct LevelEndComp {
  int NextLevelId = -1;
};

struct DoorComp {
  bool IsOpen = false;
  std::optional<int> KeyId;

  bool hasLock() const { return KeyId.has_value(); }
  bool isLocked() const { return hasLock() && !IsOpen; }
};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_LEVEL_H