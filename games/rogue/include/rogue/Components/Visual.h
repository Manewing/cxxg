#ifndef ROGUE_COMPONENTS_VISUAL_H
#define ROGUE_COMPONENTS_VISUAL_H

#include <rogue/Tile.h>

namespace rogue {

struct TileComp {
  Tile T;
  int ZIndex = 0;
};

struct NameComp {
  std::string Name;
};

}

#endif // #ifndef ROGUE_COMPONENTS_VISUAL_H