#ifndef ROGUE_COMPONENTS_VISUAL_H
#define ROGUE_COMPONENTS_VISUAL_H

#include <rogue/Tile.h>

namespace rogue {

struct TileComp {
  Tile T;

  /// Higher z-index means closer to user (overlaps)
  int ZIndex = 0;
};

struct NameComp {
  std::string Name;
};

struct CursorComp {};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_VISUAL_H