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

LevelGenerator::LevelGenerator(const GameContext &Ctx) : Ctx(Ctx) {}

void LevelGenerator::spawnEntities(const LevelEntityConfig &Cfg,
                                   Level &L) const {
  auto &EntitiesMap = L.Map.get(Level::LayerEntitiesIdx);
  const auto AllEntitiesPos = EntitiesMap.findTilesNot(Level::EmptyTile);
  for (const auto &EntityPos : AllEntitiesPos) {
    spawnEntity(EntitiesMap.getTile(EntityPos), Cfg, L, EntityPos);
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

int createNewKey(ItemDatabase &ItemDb) {
  auto KeyTemplateId = ItemDb.getItemId("Key");
  auto KeyTemplate = ItemDb.getItemProto(KeyTemplateId);
  KeyTemplate.ItemId = ItemDb.getNewItemId();
  KeyTemplate.Name = "Key " + std::to_string(KeyTemplate.ItemId);
  ItemDb.addItemProto(KeyTemplate);
  return KeyTemplate.ItemId;
}

} // namespace

void LevelGenerator::spawnEntity(Tile T, const LevelEntityConfig &Cfg, Level &L,
                                 ymir::Point2d<int> Pos) const {

  // Deal with creating creatures
  if (auto It = Cfg.Creatures.find(T.kind()); It != Cfg.Creatures.end()) {
    auto CId = Ctx.CreatureDb.getCreatureId(It->second.Name);
    const auto &CInfo = Ctx.CreatureDb.getCreature(CId);
    if (CInfo.Faction == FactionKind::Nature) {
      if (CInfo.Race == RaceKind::Dummy) {
        createDummyCreature(
            L.Reg, Pos, T, CInfo.Name,
            generateLootInventory(Ctx.ItemDb, It->second.LootTableName),
            CInfo.Stats);
      } else {
        createHostileCreature(
            L.Reg, Pos, T, CInfo.Name,
            generateLootInventory(Ctx.ItemDb, It->second.LootTableName),
            CInfo.Stats);
      }
    } else {
      createEnemy(L.Reg, Pos, T, CInfo.Name,
                  generateLootInventory(Ctx.ItemDb, It->second.LootTableName),
                  CInfo.Stats, CInfo.Faction, CInfo.Race);
    }
    return;
  }

  // Deal with creating chests
  if (auto It = Cfg.Chests.find(T.kind()); It != Cfg.Chests.end()) {
    createChestEntity(
        L.Reg, Pos, T,
        generateLootInventory(Ctx.ItemDb, It->second.LootTableName));
    return;
  }

  // Deal with creating dungeon entries
  if (auto It = Cfg.Dungeons.find(T.kind()); It != Cfg.Dungeons.end()) {
    return createWorldEntry(L.Reg, Pos, T, It->second.LevelName);
  }

  // Deal with creating locked doors
  if (auto It = Cfg.LockedDoors.find(T.kind()); It != Cfg.LockedDoors.end()) {
    auto KeyId = Ctx.ItemDb.getItemId(It->second.KeyName);
    createDoorEntity(L.Reg, Pos, T, /*IsOpen=*/false, KeyId);
    return;
  }
  // FIXME allow creating keys on the fly
  (void)createNewKey;

  // FIXME also have those in the config
  switch (T.kind()) {
  case 'H': {
    int PrevLevelId = L.getLevelId() - 1;
    createLevelEntryExit(L.Reg, Pos, T, /*IsExit=*/true, PrevLevelId);
  } break;
  case '<': {
    int NextLevelId = L.getLevelId() + 1;
    createLevelEntryExit(L.Reg, Pos, T, /*IsExit=*/false, NextLevelId);
  } break;
  case 'h': {
    createHealerEntity(L.Reg, Pos, T);
  } break;
  case 'S': {
    createShopEntity(L.Reg, Pos, T);
  } break;
  case 'w': {
    createWorkbenchEntity(L.Reg, Pos, T);
  } break;
  case '+': {
    createDoorEntity(L.Reg, Pos, T, /*IsOpen=*/false, {});
  } break;
  case '/': {
    createDoorEntity(L.Reg, Pos, T, /*IsOpen=*/true, {});
  } break;
  default:
    throw std::runtime_error("Invalid entity kind: " +
                             std::string(1, T.kind()));
    break;
  }
}

EmptyLevelGenerator::EmptyLevelGenerator(const GameContext &Ctx,
                                         const Config &Cfg)
    : LevelGenerator(Ctx), Cfg(Cfg) {}

std::shared_ptr<Level> EmptyLevelGenerator::generateLevel(int LevelId) const {
  auto NewLevel = std::make_shared<Level>(LevelId, Cfg.Size);

  NewLevel->Reg.ctx().emplace<GameContext>(Ctx);

  return NewLevel;
}

DesignedMapLevelGenerator::DesignedMapLevelGenerator(const GameContext &Ctx,
                                                     const Config &Cfg)
    : LevelGenerator(Ctx), Cfg(Cfg) {}

std::shared_ptr<Level>
DesignedMapLevelGenerator::generateLevel(int LevelId) const {
  // Create the new level
  auto NewLevel = createNewLevel(LevelId);

  // Deal with spawning entities
  spawnEntities(Cfg.EntityConfig, *NewLevel);

  NewLevel->Reg.ctx().emplace<GameContext>(Ctx);

  return NewLevel;
}

std::shared_ptr<Level>
DesignedMapLevelGenerator::createNewLevel(int LevelId) const {
  auto Map = ymir::loadMap(Cfg.MapFile);
  auto NewLevel = std::make_shared<Level>(LevelId, Map.getSize());
  auto &LevelMap = NewLevel->Map;

  // Setup the default background
  LevelMap.get(Cfg.DefaultChar.Layer).fill(Cfg.DefaultChar.T);

  // Update level map from the character encoding in the designed map
  Map.forEach([this, &LevelMap](auto Pos, auto Char) {
    auto It = Cfg.CharInfoMap.find(Char);
    if (It == Cfg.CharInfoMap.end()) {
      throw std::out_of_range("Could not find char info for char: " +
                              std::string(1, Char));
    }
    auto &[T, Layer] = It->second;
    LevelMap.get(Layer).setTile(Pos, T);
  });

  return NewLevel;
}

bool GeneratedMapLevelGenerator::DebugRooms = false;

GeneratedMapLevelGenerator::GeneratedMapLevelGenerator(const GameContext &Ctx,
                                                       const Config &Cfg)
    : LevelGenerator(Ctx), Cfg(Cfg) {}

std::shared_ptr<Level>
GeneratedMapLevelGenerator::generateLevel(int LevelId) const {
  // Create the new level
  auto NewLevel = createNewLevel(LevelId);

  // Deal with spawning entities
  spawnEntities(Cfg.EntityConfig, *NewLevel);

  NewLevel->Reg.ctx().emplace<GameContext>(Ctx);

  return NewLevel;
}

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

std::shared_ptr<Level>
createNewLevelWithGenerator(const std::filesystem::path &MapConfig,
                            unsigned Seed, int LevelId, bool DebugRooms,
                            unsigned Retries = 0) {
  auto DngCfg = loadConfigurationFile(MapConfig);
  Seed = (Seed + LevelId) ^ (Retries << 16);
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
    if (DebugRooms) {
      auto &M = NewLevel->Map.get(Level::LayerWallsDecoIdx);
      Tile DbgTile{{'0', cxxg::types::RgbColor{0, 60, 255, true, 100, 80, 50}}};
      M.getTile({0, 0}).T.Char = 'A' + Ctx.Hallways.size();
      for (const auto &Room : Ctx.Rooms) {
        M.getTile(Room.Pos) = DbgTile;
        DbgTile.T.Char++;
      }
      DbgTile.T.Char = 'a';
      for (const auto &Hallway : Ctx.Hallways) {
        M.fillRect(DbgTile, Hallway.Rect);
        DbgTile.T.Char++;
      }
    }
  } catch (const std::exception &E) {
    auto RenderedLevelMap = NewLevel->Map.render();
    ymir::Map<cxxg::types::ColoredChar, int> Map(RenderedLevelMap.getSize());
    Map.forEach([&RenderedLevelMap](auto Pos, auto &Tile) {
      Tile = RenderedLevelMap.getTile(Pos).T;
    });
    std::cerr << "std::exception: " << E.what() << std::endl
              << "Seed was: " << Seed << std::endl
              << "LevelId was: " << LevelId << std::endl
              << "Config was: " << MapConfig << std::endl
              << "Retries: " << Retries << std::endl
              << "Map: " << Map << "\033[2J" << std::endl;
    if (Retries > 10) {
      throw E;
    }
    return createNewLevelWithGenerator(MapConfig, Seed, LevelId, DebugRooms,
                                       Retries + 1);
  }
  return NewLevel;
}

} // namespace

std::shared_ptr<Level>
GeneratedMapLevelGenerator::createNewLevel(int LevelId) const {
  return createNewLevelWithGenerator(Cfg.MapConfig, Cfg.Seed, LevelId,
                                     DebugRooms);
}

CompositeMultiLevelGenerator::CompositeMultiLevelGenerator(
    const GameContext &Ctx)
    : LevelGenerator(Ctx) {}

const LevelGenerator &
CompositeMultiLevelGenerator::getGeneratorForLevel(std::size_t LevelIdx) const {
  for (const auto &[LvlEndIdx, Generator] : Generators) {
    if (LvlEndIdx >= LevelIdx) {
      return *Generator;
    }
  }
  throw std::out_of_range("No level config for level index " +
                          std::to_string(LevelIdx));
}

void CompositeMultiLevelGenerator::addGenerator(
    std::shared_ptr<LevelGenerator> Generator, std::size_t LevelEndIdx) {
  if (!Generators.empty() && Generators.back().LevelEndIdx >= LevelEndIdx) {
    throw std::out_of_range("LevelEndIdx must be increasing");
  }
  Generators.push_back({LevelEndIdx, Generator});
}

std::shared_ptr<Level>
CompositeMultiLevelGenerator::generateLevel(int LevelId) const {
  const auto &Generator = getGeneratorForLevel(LevelId);
  return Generator.generateLevel(LevelId);
}

std::size_t CompositeMultiLevelGenerator::getMaxLevelIdx() const {
  if (Generators.empty()) {
    return 0;
  }
  return Generators.back().LevelEndIdx + 1;
}

namespace {

DesignedMapLevelGenerator::Config::CharInfo
parseCharInfo(const rapidjson::Value &V) {
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
  return DesignedMapLevelGenerator::Config::CharInfo{T, Layer};
}

LevelEntityConfig loadLevelEntityConfigFromJSON(const rapidjson::Value &V) {
  LevelEntityConfig Cfg;

  // Load level specific creature configuration
  for (const auto &C : V["creatures"].GetArray()) {
    auto Key = std::string(C["key"].GetString());
    assert(Key.size() == 1);
    LevelEntityConfig::Creature CreatureCfg;
    CreatureCfg.Name = C["name"].GetString();
    CreatureCfg.LootTableName = C["loot"].GetString();
    Cfg.Creatures[Key[0]] = CreatureCfg;
  }

  // Load level specific chest configuration
  for (const auto &C : V["chests"].GetArray()) {
    auto Key = std::string(C["key"].GetString());
    assert(Key.size() == 1);
    LevelEntityConfig::Chest ChestCfg;
    ChestCfg.LootTableName = C["loot"].GetString();
    Cfg.Chests[Key[0]] = ChestCfg;
  }

  // Load level specific dungeon configuration
  for (const auto &C : V["dungeons"].GetArray()) {
    auto Key = std::string(C["key"].GetString());
    assert(Key.size() == 1);
    LevelEntityConfig::WorldEntry WE;
    WE.LevelName = C["level_name"].GetString();
    Cfg.Dungeons[Key[0]] = WE;
  }

  // Load level specific locked door configuration
  for (const auto &C : V["locked_doors"].GetArray()) {
    auto Key = std::string(C["key"].GetString());
    assert(Key.size() == 1);
    LevelEntityConfig::LockedDoor LD;
    LD.KeyName = C["key_name"].GetString();
    Cfg.LockedDoors[Key[0]] = LD;
  }

  return Cfg;
}

ymir::Size2d<int> loadSizeJSON(const rapidjson::Value &V) {
  ymir::Size2d<int> Ret;
  Ret.H = V["height"].GetInt();
  Ret.W = V["width"].GetInt();
  return Ret;
}

EmptyLevelGenerator::Config
loadEmptyMapGeneratorConfig(const std::filesystem::path &BasePath,
                            const rapidjson::Value &Doc) {
  (void)BasePath;
  EmptyLevelGenerator::Config MapCfg;
  MapCfg.Size = loadSizeJSON(Doc["map"]);
  return MapCfg;
}

DesignedMapLevelGenerator::Config
loadDesignedMapLevelGeneratorConfig(const std::filesystem::path &BasePath,
                                    const rapidjson::Value &Doc) {
  DesignedMapLevelGenerator::Config MapCfg;
  MapCfg.EntityConfig = loadLevelEntityConfigFromJSON(Doc);

  auto MapJson = Doc["map"].GetObject();
  MapCfg.MapFile = BasePath / MapJson["map_file"].GetString();

  // Load the character mapping
  for (const auto &[K, V] : MapJson["char_info"].GetObject()) {
    const auto Char = std::string_view(K.GetString())[0];
    MapCfg.CharInfoMap[Char] = parseCharInfo(V);
  }
  MapCfg.DefaultChar = parseCharInfo(MapJson["default_char"]);

  return MapCfg;
}

GeneratedMapLevelGenerator::Config
loadGeneratedMapLevelGeneratorConfig(const std::filesystem::path &BasePath,
                                     const rapidjson::Value &Doc) {
  GeneratedMapLevelGenerator::Config MapCfg;
  MapCfg.EntityConfig = loadLevelEntityConfigFromJSON(Doc);

  auto MapJson = Doc["map"].GetObject();
  MapCfg.MapConfig = BasePath / MapJson["config"].GetString();
  return MapCfg;
}

CompositeMultiLevelGenerator::Config
loadCompositeMultiLevelGeneratorConfig(const std::filesystem::path &BasePath,
                                       const rapidjson::Value &Doc) {
  CompositeMultiLevelGenerator::Config MapCfg;

  auto MapJson = Doc["map"].GetObject();
  for (const auto &Lvl : MapJson["levels"].GetArray()) {
    auto LevelEndIdx = Lvl["end"].GetUint();
    auto LevelCfg = BasePath / Lvl["config"].GetString();
    MapCfg.Levels.push_back({LevelEndIdx, LevelCfg});
  }

  return MapCfg;
}

} // namespace

LevelGeneratorLoader::LevelGeneratorLoader(const GameContext &Ctx) : Ctx(Ctx) {}

LevelGeneratorLoader::LevelConfig
LevelGeneratorLoader::loadCfg(unsigned Seed,
                              const std::filesystem::path &CfgFile) {
  LevelConfig LvlCfg;

  const auto BasePath = CfgFile.parent_path();
  const auto SchemaFile = BasePath.parent_path() / "level_config_schema.json";
  auto [DocStr, Doc] = loadJSON(CfgFile, &SchemaFile);

  auto MapCfg = Doc["map"].GetObject();
  const auto MapCfgType = std::string(MapCfg["type"].GetString());
  if (MapCfgType == "empty") {
    return loadEmptyMapGeneratorConfig(BasePath, Doc);
  }
  if (MapCfgType == "designed") {
    return loadDesignedMapLevelGeneratorConfig(BasePath, Doc);
  }
  if (MapCfgType == "generated") {
    auto Cfg = loadGeneratedMapLevelGeneratorConfig(BasePath, Doc);
    Cfg.Seed = Seed;
    return Cfg;
  }
  if (MapCfgType == "composite") {
    return loadCompositeMultiLevelGeneratorConfig(BasePath, Doc);
  }

  throw std::out_of_range("Invalid map type: " + MapCfgType);
}

std::shared_ptr<LevelGenerator>
LevelGeneratorLoader::create(unsigned Seed, const LevelConfig &Cfg) {
  if (auto *GenCfg = std::get_if<GeneratedMapLevelGenerator::Config>(&Cfg)) {
    return std::make_shared<GeneratedMapLevelGenerator>(Ctx, *GenCfg);
  }
  if (auto *DesCfg = std::get_if<DesignedMapLevelGenerator::Config>(&Cfg)) {
    return std::make_shared<DesignedMapLevelGenerator>(Ctx, *DesCfg);
  }
  if (auto *EmptyCfg = std::get_if<EmptyLevelGenerator::Config>(&Cfg)) {
    return std::make_shared<EmptyLevelGenerator>(Ctx, *EmptyCfg);
  }
  if (auto *CompCfg = std::get_if<CompositeMultiLevelGenerator::Config>(&Cfg)) {
    auto CompGen = std::make_shared<CompositeMultiLevelGenerator>(Ctx);
    for (const auto &[LevelEndIdx, LevelCfg] : CompCfg->Levels) {
      CompGen->addGenerator(load(Seed, LevelCfg), LevelEndIdx);
    }
    return CompGen;
  }
  throw std::out_of_range("Invalid map type");
}

std::shared_ptr<LevelGenerator>
LevelGeneratorLoader::load(unsigned Seed,
                           const std::filesystem::path &CfgFile) {
  auto Cfg = loadCfg(Seed, CfgFile);
  return create(Seed, Cfg);
}

} // namespace rogue