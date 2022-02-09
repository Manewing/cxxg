#include "LevelGenerator.h"
#include "Parser.h"
#include <ymir/Dungeon/BuilderPass.hpp>
#include <ymir/Dungeon/CaveRoomGenerator.hpp>
#include <ymir/Dungeon/CelAltMapFiller.hpp>
#include <ymir/Dungeon/FilterPlacer.hpp>
#include <ymir/Dungeon/LoopPlacer.hpp>
#include <ymir/Dungeon/MapFiller.hpp>
#include <ymir/Dungeon/RandomRoomGenerator.hpp>
#include <ymir/Dungeon/RectRoomGenerator.hpp>
#include <ymir/Dungeon/RoomEntityPlacer.hpp>
#include <ymir/Dungeon/RoomPlacer.hpp>
#include <ymir/Dungeon/StartEndPlacer.hpp>

template <typename TileType, typename TileCord, typename RandEngType>
void registerBuilders(ymir::Dungeon::BuilderPass &Pass) {
  using T = TileType;
  using U = TileCord;
  using RE = RandEngType;
  Pass.registerBuilder<ymir::Dungeon::CaveRoomGenerator<T, U, RE>>();
  Pass.registerBuilder<ymir::Dungeon::RectRoomGenerator<T, U, RE>>();
  Pass.registerBuilder<ymir::Dungeon::RandomRoomGenerator<T, U, RE>>();
  Pass.registerBuilder<ymir::Dungeon::RoomEntityPlacer<T, U, RE>>();
  Pass.registerBuilder<ymir::Dungeon::RoomPlacer<T, U, RE>>();
  Pass.registerBuilder<ymir::Dungeon::LoopPlacer<T, U, RE>>();
  Pass.registerBuilder<ymir::Dungeon::MapFiller<T, U>>();
  Pass.registerBuilder<ymir::Dungeon::CelAltMapFiller<T, U, RE>>();
  Pass.registerBuilder<ymir::Dungeon::StartEndPlacer<T, U, RE>>();
  Pass.registerBuilder<ymir::Dungeon::FilterPlacer<T, U, RE>>();
}

std::shared_ptr<Level> LevelGenerator::generateLevel(unsigned Seed) {
  using namespace cxxg::types;

  // Load level configuration file
  auto Cfg = loadConfigurationFile("level.cfg");
  Cfg["dungeon/seed"] = Seed;

  // Create new builder pass and register builders at it
  ymir::Dungeon::BuilderPass Pass;
  registerBuilders<Tile, int, ymir::WyHashRndEng>(Pass);

  for (auto const &[Alias, Builder] :
       Cfg.getSubDict("builder_alias/").toVec<std::string>()) {
    Pass.setBuilderAlias(Builder, Alias);
  }
  Pass.setSequence(Cfg.asList<std::string>("sequence/"));
  Pass.configure(Cfg);

  const auto Layers = Cfg.asList<std::string>("layers/");
  const auto Size = Cfg.get<ymir::Size2d<int>>("dungeon/size");
  auto NewLevel = std::make_shared<Level>(Layers, Size);
  ymir::Dungeon::Context<Tile, int> Ctx(NewLevel->Map);

  Pass.init(Ctx);
  Pass.run(Ctx);

  spawnEnemies(*NewLevel);

  return NewLevel;
}

void LevelGenerator::spawnEnemies(Level &L) {
  auto &EnemyMap = L.Map.get("enemies");
  const auto AllEnemyPos = EnemyMap.findTilesNot(Level::EmptyTile);
  for (const auto &EnemyPos : AllEnemyPos) {
    spawnEnemy(L, EnemyPos, EnemyMap.getTile(EnemyPos));
  }
  EnemyMap.fill(Level::EmptyTile);
}

void LevelGenerator::spawnEnemy(Level &L, ymir::Point2d<int> Pos, Tile T) {
  L.Entities.push_back(std::make_shared<EnemyEntity>(Pos, T));
}