#ifndef ROGUE_COMPONENTS_VISUAL_H
#define ROGUE_COMPONENTS_VISUAL_H

#include <entt/entt.hpp>
#include <rogue/Tile.h>
#include <ymir/Types.hpp>

namespace rogue {

struct TileComp {
  Tile T;
};

struct NameComp {
  std::string Name;
  std::string Description;
};

struct CursorComp {};

entt::entity createCursor(entt::registry &Reg, ymir::Point2d<int> Pos);

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_VISUAL_H