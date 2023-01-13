#ifndef ROGUE_RENDERER_H
#define ROGUE_RENDERER_H

#include <rogue/Tile.h>
#include <cxxg/Types.h>
#include <ymir/Map.hpp>
#include <ymir/Types.hpp>

namespace rogue {
class Level;
}

namespace rogue {

class Renderer {
public:
  Renderer(ymir::Size2d<int> Size, Level &L, ymir::Point2d<int> Center);

  void renderShadow(unsigned char Darkness);
  void renderFogOfWar(const ymir::Map<bool, int> &SeenMap);
  void renderLineOfSight(ymir::Point2d<int> AtPos, unsigned int Range);
  void renderVisible(ymir::Point2d<int> AtPos);

  const ymir::Map<cxxg::types::ColoredChar> &get() const { return VisibleMap; }

protected:
  void renderEntities();

private:
  Level &L;
  ymir::Point2d<int> Offset = {0, 0};
  ymir::Map<Tile> RenderedLevelMap;
  ymir::Map<cxxg::types::ColoredChar> VisibleMap;
};

} // namespace rogue

#endif // #ifndef ROGUE_RENDERER_H