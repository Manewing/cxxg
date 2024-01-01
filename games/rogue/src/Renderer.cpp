#include <rogue/Components/LOS.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Level.h>
#include <rogue/Renderer.h>
#include <ymir/Algorithm/LineOfSight.hpp>

namespace rogue {

Renderer::Renderer(ymir::Size2d<int> Size, Level &L, ymir::Point2d<int> Center)
    : L(L), VisibleMap(Size), IsVisibleMap(Size) {
  Offset.X = -(Center.X - Size.W / 2);
  Offset.Y = -(Center.Y - Size.H / 2);

  // Render map and copy to visible map
  RenderedLevelMap = L.Map.render();

  // Render background color
  L.Map.get(Level::LayerGroundIdx).forEach([this](auto Pos, auto &Tile) {
    auto *GroundColor = std::get_if<cxxg::types::RgbColor>(&Tile.color());
    if (!GroundColor) {
      return;
    }
    auto &RenderedTile = RenderedLevelMap.getTile(Pos);
    if (auto *RgbColor =
            std::get_if<cxxg::types::RgbColor>(&RenderedTile.color())) {
      if (!RgbColor->HasBackground) {
        RgbColor->HasBackground = true;
        RgbColor->BgR = GroundColor->BgR;
        RgbColor->BgG = GroundColor->BgG;
        RgbColor->BgB = GroundColor->BgB;
      }
    }
  });

  VisibleMap.fill(Level::WallTile.T);
  IsVisibleMap.fill(true);

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
  IsVisibleMap.fill(false);
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

void Renderer::renderAllLineOfSight() {
  auto View = L.Reg.view<const PositionComp, const LineOfSightComp,
                         const VisibleLOSComp>();
  View.each([this](const auto &Pos, const auto &LOS, const auto &) {
    renderLineOfSight(Pos, LOS.LOSRange);
  });
}

void Renderer::renderLineOfSight(ymir::Point2d<int> AtPos, unsigned int Range) {
  renderVisible(AtPos);

  ymir::Algorithm::shadowCasting<int>(
      [this](auto Pos) { renderVisible(Pos); },
      [this](auto Pos) { return L.isLOSBlocked(Pos); }, AtPos, Range);
}

void Renderer::renderVisible(ymir::Point2d<int> AtPos) {
  if (!VisibleMap.contains(AtPos + Offset) ||
      !RenderedLevelMap.contains(AtPos)) {
    return;
  }
  IsVisibleMap.getTile(AtPos + Offset) = true;
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

void Renderer::renderVisibleChar(const cxxg::types::ColoredChar &EffC,
                                 ymir::Point2d<int> AtPos) {
  if (!VisibleMap.contains(AtPos + Offset)) {
    return;
  }
  VisibleMap.getTile(AtPos + Offset).Char = EffC.Char;
  if (auto *RgbColor = std::get_if<cxxg::types::RgbColor>(&EffC.Color)) {
    if (RgbColor->HasBackground) {
      VisibleMap.getTile(AtPos + Offset).Color = EffC.Color;
    } else if (auto *VRgb = std::get_if<cxxg::types::RgbColor>(
                   &VisibleMap.getTile(AtPos + Offset).Color)) {
      VRgb->R = RgbColor->R;
      VRgb->G = RgbColor->G;
      VRgb->B = RgbColor->B;
    }
  } else {
    VisibleMap.getTile(AtPos + Offset).Color = EffC.Color;
  }
}

void Renderer::renderEffect(cxxg::types::ColoredChar EffC,
                            ymir::Point2d<int> AtPos) {
  renderVisibleChar(EffC, AtPos);
}

void Renderer::renderEntities() {
  L.Reg.sort<TileComp>(
      [](const auto &Lhs, const auto &Rhs) { return Lhs.ZIndex < Rhs.ZIndex; });
  L.Reg.sort<PositionComp, TileComp>();
  auto View =
      L.Reg.view<const PositionComp, const TileComp, const VisibleComp>();
  View.each([this](const auto &PC, const auto &T, const auto &VC) {
    renderVisibleEntity(PC, T, VC);
  });
}

void Renderer::renderVisibleEntity(const PositionComp &PC, const TileComp &T,
                                   const VisibleComp &VC) {
  if (!VC.IsVisible && !VC.Partially) {
    return;
  }
  if (!IsVisibleMap.contains(PC.Pos + Offset) ||
      !IsVisibleMap.getTile(PC.Pos + Offset)) {
    return;
  }
  if (!VC.IsVisible && VC.Partially) {
    auto ColorChar = T.T.T;
    ColorChar.Color = cxxg::types::RgbColor{30, 30, 30};
    renderVisibleChar(ColorChar, PC.Pos);
  } else {
    renderVisibleChar(T.T.T, PC.Pos);
  }
}

} // namespace rogue