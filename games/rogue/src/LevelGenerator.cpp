#include <rogue/Components/Entity.h>
#include <rogue/Components/RaceFaction.h>
#include <rogue/Context.h>
#include <rogue/CreatureDatabase.h>
#include <rogue/ItemDatabase.h>
#include <rogue/LevelGenerator.h>
#include <rogue/Parser.h>
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

namespace rogue {

namespace {

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

} // namespace

LevelGenerator::LevelGenerator(GameContext *Ctx) : Ctx(Ctx) {}

std::shared_ptr<Level>
LevelGenerator::generateLevel(unsigned Seed, int LevelId,
                              const std::filesystem::path &LevelConfig) {
  using namespace cxxg::types;

  // Load level configuration file
  auto Cfg = loadConfigurationFile(LevelConfig);
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
  if (Ctx) {
    NewLevel->Reg.ctx().emplace<GameContext>(*Ctx);
  }
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

namespace {

/// Generates an inventory with random loot including all items from the item
/// database
Inventory generateRandomLootInventory(const ItemDatabase &ItemDb,
                                      const unsigned MaxNumItems = 5,
                                      const unsigned MinNumItems = 1) {
  // TODO what kind of loot is there?
  //  special items based on entity kind (e.g. boss)
  //  common items based on entity level
  assert(MaxNumItems >= MinNumItems);

  Inventory Inv;

  unsigned NumItems = rand() % (MaxNumItems - MinNumItems + 1) + MinNumItems;
  for (unsigned I = 0; I < NumItems; ++I) {
    auto It = ItemDb.createItem(ItemDb.getRandomItemId());
    It.StackSize = rand() % (It.getMaxStackSize() / 2 + 1) + 1;
    Inv.addItem(It);
  }

  return Inv;
}

} // namespace

void LevelGenerator::spawnEntity(Level &L, ymir::Point2d<int> Pos, Tile T) {
  // FIXME keys to creature name
  static const std::map<char, std::string> CreatureNames = {
      {'b', "Blob"},
      {'s', "Skeleton"},
      {'t', "Troll"},
  };

  if (auto It = CreatureNames.find(T.kind()); It != CreatureNames.end()) {
    auto CId = Ctx->CreatureDb.getCreatureId(It->second);
    const auto &CInfo = Ctx->CreatureDb.getCreature(CId);
    if (CInfo.Faction == FactionKind::Nature) {
      createHostileCreature(L.Reg, Pos, T, CInfo.Name, CInfo.Stats);
    } else {
      createEnemy(L.Reg, Pos, T, CInfo.Name,
                  generateRandomLootInventory(Ctx->ItemDb), CInfo.Stats,
                  CInfo.Faction, CInfo.Race);
    }
    return;
  }

  switch (T.kind()) {
  case 'H': {
    int PrevLevelId = L.getLevelId() - 1;
    createLevelEntryExit(L.Reg, Pos, T, /*IsExit=*/true, PrevLevelId);
  } break;
  case '<': {
    int NextLevelId = L.getLevelId() + 1;
    createLevelEntryExit(L.Reg, Pos, T, /*IsExit=*/false, NextLevelId);
  } break;
  case 'C':
    createChestEntity(L.Reg, Pos, T, generateRandomLootInventory(Ctx->ItemDb));
    break;
  default:
    throw std::runtime_error("Invalid entity kind: " +
                             std::to_string(T.kind()));
    break;
  }
}

} // namespace rogue