#include "Game.h"
#include <cmath>
#include <cxxg/Row.h>
#include <cxxg/Screen.h>
#include <cxxg/Types.h>
#include <cxxg/Utils.h>
#include <ymir/Algorithm/LineOfSight.hpp>
#include <ymir/Map.hpp>
#include <ymir/Terminal.hpp>

template <typename T, typename U>
cxxg::Screen &operator<<(cxxg::Screen &Scr, const ymir::Map<T, U> &Map) {
  for (auto PY = 0; PY < Map.getSize().H; PY++) {
    for (auto PX = 0; PX < Map.getSize().W; PX++) {
      Scr[PY][PX] = Map.getTile({PX, PY});
    }
  }
  return Scr;
}

Game::Game(cxxg::Screen &Scr)
    : cxxg::Game(Scr), LevelMap(80, 24), VisibleMap(80, 24) {}

void Game::initialize(bool BufferedInput, unsigned TickDelayUs) {
  // FIXME
  PlayerPos = {37, 11};

  const cxxg::types::ColoredChar WallChar('#',
                                          cxxg::types::RgbColor{255, 255, 255});
  const cxxg::types::ColoredChar GroundChar(' ');
  LevelMap.fill(WallChar);
  LevelMap.fillRect(GroundChar, ymir::Rect2d<int>({36, 10}, {8, 4}));
  LevelMap.fillRect(GroundChar, ymir::Rect2d<int>({44, 13}, {12, 1}));
  LevelMap.fillRect(GroundChar, ymir::Rect2d<int>({46, 12}, {12, 8}));
  LevelMap.fillRect(GroundChar, ymir::Rect2d<int>({30, 11}, {6, 1}));
  LevelMap.fillRect(GroundChar, ymir::Rect2d<int>({20, 10}, {10, 4}));

  cxxg::Game::initialize(BufferedInput, TickDelayUs);
  handleDraw();
}

void Game::handleInput(int Char) {
  switch (Char) {
  case cxxg::utils::KEY_LEFT:
    movePlayer(ymir::Dir2d::LEFT);
    break;
  case cxxg::utils::KEY_RIGHT:
    movePlayer(ymir::Dir2d::RIGHT);
    break;
  case cxxg::utils::KEY_DOWN:
    movePlayer(ymir::Dir2d::DOWN);
    break;
  case cxxg::utils::KEY_UP:
    movePlayer(ymir::Dir2d::UP);
    break;
  case cxxg::utils::KEY_SPACE:
    break;
  default:
    break;
  }
}

void Game::handleDraw() {
  // Render line of sight
  renderShadow(/*Darkness=*/30);
  renderLineOfSight(PlayerPos, /*Range=*/8);

  // renderLineOfSight({50, 14}, /*Range=*/5);

  // Draw map
  Scr << VisibleMap;

  // Draw player
  const cxxg::types::ColoredChar PlayerChar(
      '@', cxxg::types::RgbColor{255, 255, 50});
  Scr[PlayerPos.Y][PlayerPos.X] = PlayerChar;

  // Draw UI overlay
  Scr[0][0] << "Rogue v0.0";

  cxxg::Game::handleDraw();
}

void Game::renderShadow(unsigned char Darkness) {
  const cxxg::types::RgbColor ShadowColor{Darkness, Darkness, Darkness};
  LevelMap.forEach([this, ShadowColor](auto Pos, const auto &Tile) {
    VisibleMap.getTile(Pos) = Tile;
    VisibleMap.getTile(Pos).Color = ShadowColor;
  });
}

void Game::renderLineOfSight(ymir::Point2d<int> AtPos, unsigned int Range) {
  ymir::Algorithm::traverseLOS(
      [this](auto Pos) -> bool {
        if (!LevelMap.contains(Pos)) {
          return false;
        }
        auto &Tile = LevelMap.getTile(Pos);
        auto &RenderTile = VisibleMap.getTile(Pos);
        switch (Tile.Char) {
        case '#':
          RenderTile.Color = cxxg::types::RgbColor{255, 255, 255};
          return false;
        case ' ':
          RenderTile.Char = '.';
          break;
        default:
          break;
        }
        return true;
      },
      AtPos, Range, 0.01);
}

void Game::movePlayer(ymir::Dir2d Dir) {
  auto NewPos = PlayerPos + Dir;
  if (LevelMap.getTile(NewPos).Char == ' ') {
    PlayerPos += Dir;
  }
}