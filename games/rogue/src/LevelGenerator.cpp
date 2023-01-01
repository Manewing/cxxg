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

#include "Components/AI.h"
#include "Components/Level.h"
#include "Components/Stats.h"
#include "Components/Transform.h"
#include "Components/Visual.h"

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
                          const std::map<char, CharInfo> &CharInfoMap) {
  auto Map = ymir::loadMap(LevelFile);
  auto NewLevel = std::make_shared<Level>(Layers, Map.getSize());
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

// FIXME move this
namespace {
void createEnemy(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                 const std::string &Name) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<HealthComp>(Entity);
  Reg.emplace<WanderAIComp>(Entity);
  Reg.emplace<TileComp>(Entity, T);
  Reg.emplace<NameComp>(Entity, Name);
  Reg.emplace<LineOfSightComp>(Entity);
  //  reg.emplace<AttackAI>(Entity);
  //  reg.emplace<MeleeAttack>(Entity, (Fac == 0 ? 60U : 20U));

  // DEBUG
  // Reg.emplace<FactionComp>(Entity, FactionKind::Enemy);
  Reg.emplace<FactionComp>(Entity, T.kind() == 't' ? FactionKind::Nature
                                                   : FactionKind::Enemy);

  Reg.emplace<AgilityComp>(Entity);
}
void createLevelEntryExit(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                          bool IsExit) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<TileComp>(Entity, T);
  if (IsExit) {
    Reg.emplace<LevelEndComp>(Entity);
  } else {
    Reg.emplace<LevelStartComp>(Entity);
  }
}
} // namespace

void LevelGenerator::spawnEntity(Level &L, ymir::Point2d<int> Pos, Tile T) {
  switch (T.kind()) {
  case 's':
    // L.Entities.push_back(std::make_shared<EnemyEntity>(Pos, T));
    createEnemy(L.Reg, Pos, T, "Skeleton");
    break;
  case 't':
    // L.Entities.push_back(std::make_shared<EnemyEntity>(Pos, T));
    createEnemy(L.Reg, Pos, T, "Troll");
    break;
  case 'H':
    // L.Entities.push_back(std::make_shared<LevelStartEntity>(Pos, T));
    createLevelEntryExit(L.Reg, Pos, T, /*IsExit=*/false);
    break;
  case '<':
    // L.Entities.push_back(std::make_shared<LevelEndEntity>(Pos, T));
    createLevelEntryExit(L.Reg, Pos, T, /*IsExit=*/true);
    break;
  case 'C':
    L.Entities.push_back(std::make_shared<ChestEntity>(Pos, T));
    break;
  }
}