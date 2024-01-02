#ifndef ROGUE_COMPONENTS_VISUAL_H
#define ROGUE_COMPONENTS_VISUAL_H

#include <entt/entt.hpp>
#include <rogue/Tile.h>
#include <ymir/Types.hpp>

namespace rogue {

struct TileComp {
  Tile T;

  /// Higher z-index means closer to user (overlaps)
  int ZIndex = 0;
};

struct NameComp {
  std::string Name;
  std::string Description;
};

struct CursorComp {};

entt::entity createCursor(entt::registry &Reg, ymir::Point2d<int> Pos);

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_VISUAL_H