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

} // namespace

void LevelGenerator::spawnEntity(Tile T, const LevelEntityConfig &Cfg, Level &L,
                                 ymir::Point2d<int> Pos) const {
  if (auto It = Cfg.Creatures.find(T.kind()); It != Cfg.Creatures.end()) {
    auto CId = Ctx.CreatureDb.getCreatureId(It->second.Name);
    const auto &CInfo = Ctx.CreatureDb.getCreature(CId);
    if (CInfo.Faction == FactionKind::Nature) {
      createHostileCreature(
          L.Reg, Pos, T, CInfo.Name,
          generateLootInventory(Ctx.ItemDb, It->second.LootTableName),
          CInfo.Stats);
    } else {
      createEnemy(L.Reg, Pos, T, CInfo.Name,
                  generateLootInventory(Ctx.ItemDb, It->second.LootTableName),
                  CInfo.Stats, CInfo.Faction, CInfo.Race);
    }
    return;
  }
  if (auto It = Cfg.Chests.find(T.kind()); It != Cfg.Chests.end()) {
    createChestEntity(
        L.Reg, Pos, T,
        generateLootInventory(Ctx.ItemDb, It->second.LootTableName));
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

} // namespace

std::shared_ptr<Level>
GeneratedMapLevelGenerator::createNewLevel(int LevelId) const {
  auto DngCfg = loadConfigurationFile(Cfg.MapConfig);
  DngCfg["dungeon/seed"] = Cfg.Seed + LevelId;

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
    std::cerr << "std::exception: " << E.what() << std::endl
              << "Seed was: " << Cfg.Seed << std::endl
              << Map << "\033[2J" << std::endl;
    throw E;
  }

  return NewLevel;
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
  throw std::out_of_range("No level config for level " +
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