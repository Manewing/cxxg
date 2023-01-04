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
#include <ymir/MapIo.hpp>

#include "Components/Entity.h"

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

std::shared_ptr<Level> LevelGenerator::generateLevel(unsigned Seed,
                                                     int LevelId) {
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
  auto NewLevel = std::make_shared<Level>(LevelId, Layers, Size);
  ymir::Dungeon::Context<Tile, int> Ctx(NewLevel->Map);

  Pass.init(Ctx);
  try {
    Pass.run(Ctx);
  } catch (const std::exception &E) {
    auto RenderedLevelMap = NewLevel->Map.render();
    ymir::Map<cxxg::types::ColoredChar, int> Map(RenderedLevelMap.getSize());
    Map.forEach([&RenderedLevelMap](auto Pos, auto &Tile) {
      Tile = RenderedLevelMap.getTile(Pos).T;
    });
    std::cout << Map << std::endl;
    throw E;
  }

  spawnEntities(*NewLevel);

  return NewLevel;
}

std::shared_ptr<Level>
LevelGenerator::loadLevel(const std::filesystem::path &LevelFile,
                          const std::vector<std::string> &Layers,
                          const std::map<char, CharInfo> &CharInfoMap,
                          int LevelId) {
  auto Map = ymir::loadMap(LevelFile);
  auto NewLevel = std::make_shared<Level>(LevelId, Layers, Map.getSize());
  auto &LevelMap = NewLevel->Map;

  Map.forEach([&LevelMap, &CharInfoMap](auto Pos, auto Char) {
    auto &[T, Layer] = CharInfoMap.at(Char);
    LevelMap.get(Layer).setTile(Pos, T);
  });
  return NewLevel;
}

void LevelGenerator::spawnEntities(Level &L) {
  auto &EntitiesMap = L.Map.get("entities");
  const auto AllEntitiesPos = EntitiesMap.findTilesNot(Level::EmptyTile);
  for (const auto &EntityPos : AllEntitiesPos) {
    spawnEntity(L, EntityPos, EntitiesMap.getTile(EntityPos));
  }
  EntitiesMap.fill(Level::EmptyTile);
}

void LevelGenerator::spawnEntity(Level &L, ymir::Point2d<int> Pos, Tile T) {
  switch (T.kind()) {
  case 's':
    createEnemy(L.Reg, Pos, T, "Skeleton");
    break;
  case 't':
    createEnemy(L.Reg, Pos, T, "Troll");
    break;
  case 'H': {
    int PrevLevelId = L.getLevelId() - 1;
    createLevelEntryExit(L.Reg, Pos, T, /*IsExit=*/true, PrevLevelId);
  } break;
  case '<': {
    int NextLevelId = L.getLevelId() + 1;
    createLevelEntryExit(L.Reg, Pos, T, /*IsExit=*/false, NextLevelId);
  } break;
  case 'C':
    L.Entities.push_back(std::make_shared<ChestEntity>(Pos, T));
    break;
  default:
    throw std::runtime_error("Invalid entity kind: " +
                             std::to_string(T.kind()));
    break;
  }
}