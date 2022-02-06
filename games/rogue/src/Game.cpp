#include "Game.h"
#include <cmath>
#include <cxxg/Row.h>
#include <cxxg/Screen.h>
#include <cxxg/Types.h>
#include <cxxg/Utils.h>
#include <gtest/gtest.h>
#include <ymir/Algorithm/LineOfSight.hpp>
#include <ymir/Config/Parser.hpp>
#include <ymir/Config/Types.hpp>
#include <ymir/Dungeon/BuilderPass.hpp>
#include <ymir/Dungeon/CaveRoomGenerator.hpp>
#include <ymir/Dungeon/CelAltMapFiller.hpp>
#include <ymir/Dungeon/ChestPlacer.hpp>
#include <ymir/Dungeon/LoopPlacer.hpp>
#include <ymir/Dungeon/MapFiller.hpp>
#include <ymir/Dungeon/RandomRoomGenerator.hpp>
#include <ymir/Dungeon/RectRoomGenerator.hpp>
#include <ymir/Dungeon/StartEndPlacer.hpp>
#include <ymir/Dungeon/FilterPlacer.hpp>
#include <ymir/Dungeon/RoomPlacer.hpp>
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
    : cxxg::Game(Scr), LevelMap(1, {80, 24}), VisibleMap(80, 24) {}

void Game::initialize(bool BufferedInput, unsigned TickDelayUs) {
  // FIXME
  PlayerPos = {37, 11};

  generateLevel(0);

  cxxg::Game::initialize(BufferedInput, TickDelayUs);
  handleDraw();
}


template <typename TileType, typename TileCord, typename RandEngType>
void registerBuilders(ymir::Dungeon::BuilderPass &Pass) {
  using T = TileType;
  using U = TileCord;
  using RE = RandEngType;
  Pass.registerBuilder<ymir::Dungeon::CaveRoomGenerator<T, U, RE>>();
  Pass.registerBuilder<ymir::Dungeon::RectRoomGenerator<T, U, RE>>();
  Pass.registerBuilder<ymir::Dungeon::RandomRoomGenerator<T, U, RE>>();
  Pass.registerBuilder<ymir::Dungeon::ChestPlacer<T, U, RE>>();
  Pass.registerBuilder<ymir::Dungeon::RoomPlacer<T, U, RE>>();
  Pass.registerBuilder<ymir::Dungeon::LoopPlacer<T, U, RE>>();
  Pass.registerBuilder<ymir::Dungeon::MapFiller<T, U>>();
  Pass.registerBuilder<ymir::Dungeon::CelAltMapFiller<T, U, RE>>();
  Pass.registerBuilder<ymir::Dungeon::StartEndPlacer<T, U, RE>>();
  Pass.registerBuilder<ymir::Dungeon::FilterPlacer<T, U, RE>>();
}

cxxg::types::ColoredChar parseColoredChar(const std::string &Value) {
  static const std::regex Regex("\\{('..?'), (\"#[0-9a-fA-F]+\")\\}");
  std::smatch Match;
  if (std::regex_match(Value, Match, Regex)) {
    auto Char = ymir::Config::Parser::parseChar(Match[1]);
    auto Color = ymir::Config::parseRgbColor(Match[2]);
    cxxg::types::RgbColor CxxColor{Color.R, Color.G, Color.B, Color.Foreground};
    std::cerr << "Parsed char: '" << Char << "'" << std::endl;
    return cxxg::types::ColoredChar(Char, CxxColor);
  }
  throw std::runtime_error("Invalid colored char format: " + Value);
}

void registerParserTypes(ymir::Config::Parser &P) {
  P.registerType("CC", parseColoredChar);
}

void Game::generateLevel(unsigned Seed) {
  using namespace cxxg::types;

   // Load configuration file
  ymir::Config::Parser CfgParser;
  ymir::Config::registerYmirTypes<int>(CfgParser);
  registerParserTypes(CfgParser);
  std::filesystem::path CfgFile = "level.cfg";
  CfgParser.parse(CfgFile);
  auto &Cfg = CfgParser.getCfg();

  Cfg["dungeon/seed"] = Seed;

  // Create new builder pass and register builders at it
  ymir::Dungeon::BuilderPass Pass;
  registerBuilders<ColoredChar, int, ymir::WyHashRndEng>(Pass);

  for (auto const &[Alias, Builder] :
       Cfg.getSubDict("builder_alias/").toVec<std::string>()) {
    Pass.setBuilderAlias(Builder, Alias);
  }
  Pass.setSequence(Cfg.getSubDict("sequence/").values<std::string>());
  Pass.configure(Cfg);

  const auto Layers = Cfg.getSubDict("layers/").values<std::string>();
  const auto Size = Cfg.get<ymir::Size2d<int>>("dungeon/size");
  ymir::LayeredMap<ColoredChar> Map(Layers, Size);
  ymir::Dungeon::Context<ColoredChar, int> Ctx(Map);

  Pass.init(Ctx);
  Pass.run(Ctx);
  LevelMap = Ctx.Map;
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
  LevelMap.render().forEach([this, ShadowColor](auto Pos, const auto &Tile) {
    VisibleMap.getTile(Pos) = Tile;
    VisibleMap.getTile(Pos).Color = ShadowColor;
  });
}

void Game::renderLineOfSight(ymir::Point2d<int> AtPos, unsigned int Range) {
  // FIXME this is shitty and inefficient
  auto RenderedLevelMap = LevelMap.render();

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
  if (LevelMap.get("walls").getTile(NewPos).Char != '#') {
    PlayerPos += Dir;
  } else {
    warn() << "Can't move";
  }
}