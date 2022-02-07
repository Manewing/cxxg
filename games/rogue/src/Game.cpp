#include "Game.h"
#include <cmath>
#include <cxxg/Row.h>
#include <cxxg/Screen.h>
#include <cxxg/Types.h>
#include <cxxg/Utils.h>
#include <gtest/gtest.h>
#include <ymir/Algorithm/LineOfSight.hpp>

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
    : cxxg::Game(Scr), LevelGen(), CurrentLevel(nullptr), VisibleMap(80, 24) {}

void Game::initialize(bool BufferedInput, unsigned TickDelayUs) {
  // FIXME
  PlayerPos = {37, 11};

  generateLevel(0);

  cxxg::Game::initialize(BufferedInput, TickDelayUs);
  handleDraw();
}

void Game::generateLevel(unsigned Seed) {
  CurrentLevel = LevelGen.generateLevel(Seed);
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
  CurrentLevel->Map.render().forEach(
      [this, ShadowColor](auto Pos, const auto &Tile) {
        VisibleMap.getTile(Pos) = Tile;
        VisibleMap.getTile(Pos).Color = ShadowColor;
      });
}

void Game::renderLineOfSight(ymir::Point2d<int> AtPos, unsigned int Range) {
  // FIXME this is shitty and inefficient
  auto RenderedLevelMap = CurrentLevel->Map.render();

  ymir::Algorithm::traverseLOS(
      [this, &RenderedLevelMap](auto Pos) -> bool {
        if (!VisibleMap.contains(Pos)) {
          return false;
        }
        auto &Tile = VisibleMap.getTile(Pos);
        auto &RenderedTile = RenderedLevelMap.getTile(Pos);
        switch (RenderedTile.Char) {
        case '#':
          Tile.Color = RenderedTile.Color;
          return false;
        case ' ':
          Tile.Char = '.';
          Tile.Color = RenderedTile.Color;
          break;
        default:
          Tile.Color = RenderedTile.Color;
          break;
        }
        return true;
      },
      AtPos, Range, 0.01);
}

void Game::movePlayer(ymir::Dir2d Dir) {
  auto NewPos = PlayerPos + Dir;
  if (!CurrentLevel->isBodyBlocked(NewPos)) {
    PlayerPos += Dir;
  } else {
    warn() << "Can't move";
  }
}