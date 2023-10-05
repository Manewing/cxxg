#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Level.h>
#include <rogue/Renderer.h>
#include <ymir/Algorithm/LineOfSight.hpp>

namespace rogue {

Renderer::Renderer(ymir::Size2d<int> Size, Level &L, ymir::Point2d<int> Center)
    : L(L), VisibleMap(Size) {
  Offset.X = -(Center.X - Size.W / 2);
  Offset.Y = -(Center.Y - Size.H / 2);

  // Render map and copy to visible map
  RenderedLevelMap = L.Map.render();
  renderEntities();

  // FIXME make this configurable
  VisibleMap.fill(Level::WallTile.T);

  VisibleMap.forEach([this](auto Pos, auto &Tile) {
    Pos -= Offset;
    if (!RenderedLevelMap.contains(Pos)) {
      return;
    }
    Tile = RenderedLevelMap.getTile(Pos).T;
  });
}

void Renderer::renderShadow(unsigned char Darkness) {
  const cxxg::types::RgbColor ShadowColor{Darkness, Darkness, Darkness};
  VisibleMap.forEach(
      [ShadowColor](auto, auto &Tile) { Tile.Color = ShadowColor; });
}

void Renderer::renderFogOfWar(const ymir::Map<bool, int> &SeenMap) {
  static constexpr Tile FogTile =
      Tile{{'#', cxxg::types::RgbColor{20, 20, 20, true, 18, 18, 18}}};
  VisibleMap.forEach([&SeenMap, this](auto Pos, auto &Tile) {
    if (!SeenMap.contains(Pos - Offset) || !SeenMap.getTile(Pos - Offset)) {
      Tile = FogTile.T;
    }
  });
  (void)FogTile;
}

void Renderer::renderLineOfSight(ymir::Point2d<int> AtPos, unsigned int Range) {
  renderVisible(AtPos);

  ymir::Algorithm::traverseLOS(
      [this](auto Pos) -> bool {
        renderVisible(Pos);
        return !L.isLOSBlocked(Pos);
      },
      AtPos, Range, 0.3, 0.01);
}

void Renderer::renderVisible(ymir::Point2d<int> AtPos) {
  if (!VisibleMap.contains(AtPos + Offset)) {
    return;
  }
  auto &Tile = VisibleMap.getTile(AtPos + Offset);
  auto &RenderedTile = RenderedLevelMap.getTile(AtPos);

  Tile.Color = RenderedTile.color();
  switch (RenderedTile.kind()) {
  case ' ': // FIXME define constants
    Tile.Char = '.';
    break;
  default:
    Tile.Char = RenderedTile.kind();
    break;
  }
}

void Renderer::renderEffect(cxxg::types::ColoredChar EffC,
                            ymir::Point2d<int> AtPos) {
  if (!VisibleMap.contains(AtPos + Offset)) {
    return;
  }
  VisibleMap.getTile(AtPos + Offset) = EffC;
}

void Renderer::renderEntities() {
  L.Reg.sort<TileComp>(
      [](const auto &Lhs, const auto &Rhs) { return Lhs.ZIndex < Rhs.ZIndex; });
  L.Reg.sort<PositionComp, TileComp>();
  auto View = L.Reg.view<const PositionComp, const TileComp>();
  View.each([this](const auto &Pos, const auto &T) {
    RenderedLevelMap.getTile(Pos) = T.T;
  });
}

} // namespace rogue