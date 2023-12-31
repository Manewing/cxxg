#ifndef ROGUE_RENDERER_H
#define ROGUE_RENDERER_H

#include <cxxg/Screen.h>
#include <cxxg/Types.h>
#include <rogue/Tile.h>
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
  void renderAllLineOfSight();
  void renderLineOfSight(ymir::Point2d<int> AtPos, unsigned int Range);
  void renderVisible(ymir::Point2d<int> AtPos);
  void renderEffect(cxxg::types::ColoredChar EffC, ymir::Point2d<int> AtPos);

  const ymir::Map<cxxg::types::ColoredChar> &get() const { return VisibleMap; }

protected:
  void renderEntities();

private:
  Level &L;
  ymir::Point2d<int> Offset = {0, 0};
  ymir::Map<Tile> RenderedLevelMap;
  ymir::Map<cxxg::types::ColoredChar> VisibleMap;
};

template <typename T, typename U>
cxxg::Screen &operator<<(cxxg::Screen &Scr, const ymir::Map<T, U> &Map) {
  for (auto PY = 0; PY < Map.getSize().H; PY++) {
    for (auto PX = 0; PX < Map.getSize().W; PX++) {
      Scr[PY][PX] = Map.getTile({PX, PY});
    }
  }
  return Scr;
}

} // namespace rogue

#endif // #ifndef ROGUE_RENDERER_H