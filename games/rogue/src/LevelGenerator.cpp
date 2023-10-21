#include <rogue/Components/Entity.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/RaceFaction.h>
#include <rogue/Context.h>
#include <rogue/CreatureDatabase.h>
#include <rogue/ItemDatabase.h>
#include <rogue/JSON.h>
#include <rogue/LevelGenerator.h>
#include <rogue/LootTable.h>
#include <rogue/Parser.h>
#include <ymir/Config/Types.hpp>
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

LevelConfig::GeneratedMap
loadGenMapConfig(const std::filesystem::path &BasePath,
                 const rapidjson::Value &MapJson) {
  LevelConfig::GeneratedMap MapCfg;
  MapCfg.Config = BasePath / MapJson["config"].GetString();
  return MapCfg;
}

LevelConfig::DesignedMap::CharInfo parseCharInfo(const rapidjson::Value &V) {
  const auto Color = ymir::Config::parseRgbColor(V["color"].GetString());
  const auto Key = std::string_view(V["key"].GetString());
  const auto Layer = std::string(V["layer"].GetString());
  cxxg::types::RgbColor CxxColor{Color.R, Color.G, Color.B};

  if (V.HasMember("bg_color")) {
    const auto BgColor = ymir::Config::parseRgbColor(V["bg_color"].GetString());
    CxxColor.HasBackground = true;
    CxxColor.BgR = BgColor.R;
    CxxColor.BgG = BgColor.G;
    CxxColor.BgB = BgColor.B;
  }

  Tile T = {{Key[0], CxxColor}};
  return LevelConfig::DesignedMap::CharInfo{T, Layer};
}

LevelConfig::DesignedMap loadDesMapConfig(const std::filesystem::path &BasePath,
                                          const rapidjson::Value &MapJson) {
  LevelConfig::DesignedMap MapCfg;
  MapCfg.MapFile = BasePath / MapJson["map_file"].GetString();

  // Load the character mapping
  for (const auto &[K, V] : MapJson["char_info"].GetObject()) {
    const auto Char = std::string_view(K.GetString())[0];
    MapCfg.CharInfoMap[Char] = parseCharInfo(V);
  }
  MapCfg.DefaultChar = parseCharInfo(MapJson["default_char"]);

  return MapCfg;
}

LevelConfig loadLevelConfig(const std::filesystem::path &LvlCfgPath) {
  LevelConfig LvlCfg;

  const auto SchemaFile =
      LvlCfgPath.parent_path().parent_path() / "level_config_schema.json";
  auto [DocStr, Doc] = loadJSON(LvlCfgPath, &SchemaFile);

  // Load level specific creature configuration
  for (const auto &CI : Doc["creatures"].GetArray()) {
    auto Key = std::string(CI["key"].GetString());
    assert(Key.size() == 1);
    LevelConfig::Creature C;
    C.Name = CI["name"].GetString();
    C.LootTableName = CI["loot"].GetString();
    LvlCfg.Creatures[Key[0]] = C;
  }

  // Load level specific chest configuration
  for (const auto &CI : Doc["chests"].GetArray()) {
    auto Key = std::string(CI["key"].GetString());
    assert(Key.size() == 1);
    LevelConfig::Chest C;
    C.LootTableName = CI["loot"].GetString();
    LvlCfg.Chests[Key[0]] = C;
  }

  auto MapCfg = Doc["map"].GetObject();
  const auto MapCfgType = std::string(MapCfg["type"].GetString());
  if (MapCfgType == "generated") {
    LvlCfg.Map = loadGenMapConfig(LvlCfgPath.parent_path(), MapCfg);
  } else {
    LvlCfg.Map = loadDesMapConfig(LvlCfgPath.parent_path(), MapCfg);
  }

  return LvlCfg;
}

std::shared_ptr<Level>
procedurallyGenerateLevel(unsigned Seed, int LevelId,
                          const LevelConfig::GeneratedMap &MapCfg) {
  auto DngCfg = loadConfigurationFile(MapCfg.Config);
  DngCfg["dungeon/seed"] = Seed;

  // Create new builder pass and register builders at it
  ymir::Dungeon::BuilderPass Pass;
  registerBuilders<Tile, int, ymir::WyHashRndEng>(Pass);

  for (auto const &[Alias, Builder] :
       DngCfg.getSubDict("builder_alias/").toVec<std::string>()) {
    Pass.setBuilderAlias(Builder, Alias);
  }
  Pass.setSequence(DngCfg.asList<std::string>("sequence/"));
  Pass.configure(DngCfg);

  const auto Size = DngCfg.get<ymir::Size2d<int>>("dungeon/size");
  auto NewLevel = std::make_shared<Level>(LevelId, Size);
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

  return NewLevel;
}

std::shared_ptr<Level> loadDesignedLevel(const LevelConfig::DesignedMap &MapCfg,
                                         int LevelId) {
  auto Map = ymir::loadMap(MapCfg.MapFile);
  auto NewLevel = std::make_shared<Level>(LevelId, Map.getSize());
  auto &LevelMap = NewLevel->Map;

  // Setup the default background
  LevelMap.get(MapCfg.DefaultChar.Layer).fill(MapCfg.DefaultChar.T);

  Map.forEach([&LevelMap, &MapCfg](auto Pos, auto Char) {
    auto &[T, Layer] = MapCfg.CharInfoMap.at(Char);
    LevelMap.get(Layer).setTile(Pos, T);
  });
  return NewLevel;
}

} // namespace

LevelGenerator::LevelGenerator(GameContext *Ctx) : Ctx(Ctx) {}

std::shared_ptr<Level>
LevelGenerator::generateLevel(unsigned Seed, int LevelId,
                              const std::filesystem::path &CfgFile) {
  // Load level configuration file
  auto Cfg = loadLevelConfig(CfgFile);
  return generateLevel(Seed, LevelId, Cfg);
}

std::shared_ptr<Level> LevelGenerator::generateLevel(unsigned Seed, int LevelId,
                                                     const LevelConfig &Cfg) {

  std::shared_ptr<Level> NewLevel;
  if (auto *GenMapCfg = std::get_if<LevelConfig::GeneratedMap>(&Cfg.Map)) {
    NewLevel = procedurallyGenerateLevel(Seed, LevelId, *GenMapCfg);
  } else {
    auto &DesMapCfg = std::get<LevelConfig::DesignedMap>(Cfg.Map);
    NewLevel = loadDesignedLevel(DesMapCfg, LevelId);
  }

  if (Ctx) {
    NewLevel->Reg.ctx().emplace<GameContext>(*Ctx);
  }

  spawnEntities(Cfg, *NewLevel);

  return NewLevel;
}

void LevelGenerator::spawnEntities(const LevelConfig &Cfg, Level &L) {
  auto &EntitiesMap = L.Map.get("entities");
  const auto AllEntitiesPos = EntitiesMap.findTilesNot(Level::EmptyTile);
  for (const auto &EntityPos : AllEntitiesPos) {
    spawnEntity(Cfg, L, EntityPos, EntitiesMap.getTile(EntityPos));
  }
  EntitiesMap.fill(Level::EmptyTile);
}

namespace {

Inventory generateLootInventory(const ItemDatabase &ItemDb,
                                const std::string &LootTableName) {
  const auto &LtCt = ItemDb.getLootTable(LootTableName);
  auto Loot = LtCt->generateLoot();

  Inventory Inv;
  for (const auto &Rw : Loot) {
    auto It = ItemDb.createItem(Rw.ItId, Rw.Count);
    Inv.addItem(It);
  }
  return Inv;
}

} // namespace

void LevelGenerator::spawnEntity(const LevelConfig &Cfg, Level &L,
                                 ymir::Point2d<int> Pos, Tile T) {
  if (auto It = Cfg.Creatures.find(T.kind()); It != Cfg.Creatures.end()) {
    auto CId = Ctx->CreatureDb.getCreatureId(It->second.Name);
    const auto &CInfo = Ctx->CreatureDb.getCreature(CId);
    if (CInfo.Faction == FactionKind::Nature) {
      createHostileCreature(L.Reg, Pos, T, CInfo.Name, CInfo.Stats);
    } else {
      createEnemy(L.Reg, Pos, T, CInfo.Name,
                  generateLootInventory(Ctx->ItemDb, It->second.LootTableName),
                  CInfo.Stats, CInfo.Faction, CInfo.Race);
    }
    return;
  }
  if (auto It = Cfg.Chests.find(T.kind()); It != Cfg.Chests.end()) {
    createChestEntity(
        L.Reg, Pos, T,
        generateLootInventory(Ctx->ItemDb, It->second.LootTableName));
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
  default:
    throw std::runtime_error("Invalid entity kind: " +
                             std::string(1, T.kind()));
    break;
  }
}

} // namespace rogue