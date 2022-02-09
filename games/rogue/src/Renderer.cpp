#include "Renderer.h"
#include "Level.h"
#include <ymir/Algorithm/LineOfSight.hpp>

Renderer::Renderer(Level &L)
    : L(L), VisibleMap(L.Map.getSize()) {
  // Render map and copy to visible map
  RenderedLevelMap = L.Map.render();
  renderEntities();
  RenderedLevelMap.forEach(
      [this](auto Pos, const auto &Tile) { VisibleMap.getTile(Pos) = Tile.T; });
}

void Renderer::renderShadow(unsigned char Darkness) {
  const cxxg::types::RgbColor ShadowColor{Darkness, Darkness, Darkness};
  VisibleMap.forEach(
      [ShadowColor](auto, auto &Tile) { Tile.Color = ShadowColor; });
}

void Renderer::renderLineOfSight(ymir::Point2d<int> AtPos, unsigned int Range) {
  renderVisible(AtPos);
  ymir::Algorithm::traverseLOS(
      [this](auto Pos) -> bool {
        if (!VisibleMap.contains(Pos)) {
          return false;
        }
        renderVisible(Pos);
        return !L.isLOSBlocked(Pos);
      },
      AtPos, Range, 0.01);
}

void Renderer::renderVisible(ymir::Point2d<int> AtPos) {
  auto &Tile = VisibleMap.getTile(AtPos);
  auto &RenderedTile = RenderedLevelMap.getTile(AtPos);

  Tile.Color = RenderedTile.color();
  switch (RenderedTile.kind()) {
  case ' ': // FIXME define constants
    Tile.Char = '.';
    break;
  default:
    break;
  }
}

void Renderer::renderEntities() {
  // FIXME enemies spawn on objects
  for (const auto *Entity : L.getEntities()) {
    RenderedLevelMap.getTile(Entity->Pos) = Entity->T;
  }
}