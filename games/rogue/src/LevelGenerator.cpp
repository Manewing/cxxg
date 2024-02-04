#include <rogue/Components/Items.h>
#include <rogue/Components/Level.h>
#include <rogue/Components/RaceFaction.h>
#include <rogue/Components/Transform.h>
#include <rogue/Context.h>
#include <rogue/EntityDatabase.h>
#include <rogue/ItemDatabase.h>
#include <rogue/JSON.h>
#include <rogue/JSONHelpers.h>
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

LevelGenerator::LevelGenerator(const GameContext &Ctx,
                               const std::filesystem::path &DataDir)
    : Ctx(Ctx), DataDir(DataDir) {}

void LevelGenerator::spawnEntities(const LevelEntityConfig &Cfg,
                                   Level &L) const {
  auto &EntitiesMap = L.Map.get(Level::LayerEntitiesIdx);
  const auto AllEntitiesPos = EntitiesMap.findTilesNot(Level::EmptyTile);
  for (const auto &EntityPos : AllEntitiesPos) {
    spawnEntity(EntitiesMap.getTile(EntityPos).kind(), Cfg, L, EntityPos);
  }
  EntitiesMap.fill(Level::EmptyTile);
}

namespace {

void spawnAndPlaceEntity(EntityFactory &Factory, ymir::Point2d<int> Pos,
                         EntityTemplateId EtId, int LevelId) {
  auto Entity = Factory.createEntity(EtId);
  auto &Reg = Factory.getRegistry();
  if (auto *PC = Reg.try_get<PositionComp>(Entity)) {
    PC->Pos = Pos;
  }
  if (auto *LSC = Reg.try_get<LevelStartComp>(Entity)) {
    LSC->NextLevelId = LevelId - 1;
  }
  if (auto *LEC = Reg.try_get<LevelEndComp>(Entity)) {
    LEC->NextLevelId = LevelId + 1;
  }
}

} // namespace

void LevelGenerator::spawnEntity(char Char, const LevelEntityConfig &Cfg,
                                 Level &L, ymir::Point2d<int> Pos) const {
  EntityFactory Factory(L.Reg, Ctx.EntityDb);
  auto It = Cfg.Entities.find(Char);
  if (It == Cfg.Entities.end()) {
    std::stringstream SS;
    SS << "Could not find entity for char: " << Char << " at " << Pos << "\n"
       << "Entities:\n";
    for (const auto &[K, V] : Cfg.Entities) {
      SS << "  " << K << " -> " << V << "\n";
    }
    throw std::out_of_range(SS.str());
  }
  spawnAndPlaceEntity(Factory, Pos,
                      Ctx.EntityDb.getEntityTemplateId(It->second),
                      L.getLevelId());
}

EmptyLevelGenerator::EmptyLevelGenerator(const GameContext &Ctx,
                                         const std::filesystem::path &DataDir,
                                         const Config &Cfg)
    : LevelGenerator(Ctx, DataDir), Cfg(Cfg) {}

std::shared_ptr<Level> EmptyLevelGenerator::generateLevel(int LevelId) const {
  auto NewLevel = std::make_shared<Level>(LevelId, Cfg.Size);

  NewLevel->Reg.ctx().emplace<GameContext>(Ctx);
  NewLevel->Reg.ctx().emplace<Level *>(NewLevel.get());

  return NewLevel;
}

DesignedMapLevelGenerator::DesignedMapLevelGenerator(
    const GameContext &Ctx, const std::filesystem::path &DataDir,
    const Config &Cfg)
    : LevelGenerator(Ctx, DataDir), Cfg(Cfg) {}

std::shared_ptr<Level>
DesignedMapLevelGenerator::generateLevel(int LevelId) const {
  // Create the new level
  auto NewLevel = createNewLevel(LevelId);

  // Deal with spawning entities
  spawnEntities(Cfg.EntityConfig, *NewLevel);

  NewLevel->Reg.ctx().emplace<GameContext>(Ctx);
  NewLevel->Reg.ctx().emplace<Level *>(NewLevel.get());

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
  Map.forEach([this, &LevelMap, &NewLevel](auto Pos, auto Char) {
    auto It = Cfg.CharInfoMap.find(Char);
    if (It == Cfg.CharInfoMap.end()) {
      spawnEntity(Char, Cfg.EntityConfig, *NewLevel, Pos);
      return;
    }
    auto &[T, Layer] = It->second;
    LevelMap.get(Layer).setTile(Pos, T);
  });

  return NewLevel;
}

bool GeneratedMapLevelGenerator::DebugRooms = false;

GeneratedMapLevelGenerator::GeneratedMapLevelGenerator(
    const GameContext &Ctx, const std::filesystem::path &DataDir,
    const Config &Cfg)
    : LevelGenerator(Ctx, DataDir), Cfg(Cfg) {}

std::shared_ptr<Level>
GeneratedMapLevelGenerator::generateLevel(int LevelId) const {
  // Create the new level
  auto NewLevel = createNewLevel(LevelId);

  // Deal with spawning entities
  spawnEntities(Cfg.EntityConfig, *NewLevel);

  NewLevel->Reg.ctx().emplace<GameContext>(Ctx);
  NewLevel->Reg.ctx().emplace<Level *>(NewLevel.get());

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
    const GameContext &Ctx, const std::filesystem::path &DataDir)
    : LevelGenerator(Ctx, DataDir) {}

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

TiledMapLevelGenerator::TiledMapLevelGenerator(
    const GameContext &Ctx, const std::filesystem::path &DataDir,
    const Config &Cfg)
    : LevelGenerator(Ctx, DataDir), Cfg(Cfg) {}

std::shared_ptr<Level>
TiledMapLevelGenerator::generateLevel(int LevelId) const {
  auto NewLevel = createNewLevel(LevelId);
  NewLevel->Reg.ctx().emplace<GameContext>(Ctx);
  NewLevel->Reg.ctx().emplace<Level *>(NewLevel.get());
  return NewLevel;
}

namespace {

ymir::LayeredMap<int, int> loadTiledMap(const std::filesystem::path &MapFile) {
  auto [DocStr, Doc] = loadJSON(MapFile, nullptr);
  auto TiledJson = Doc.GetObject();

  auto Width = TiledJson["width"].GetInt();
  auto Height = TiledJson["height"].GetInt();

  auto Layers = TiledJson["layers"].GetArray();

  std::vector<std::string> LayerNames;
  for (const auto &Layer : Layers) {
    auto LayerName = std::string(Layer["name"].GetString());
    LayerNames.push_back(LayerName);
  }

  ymir::LayeredMap<int, int> Map(LayerNames, ymir::Size2d<int>{Width, Height});

  for (const auto &Layer : Layers) {
    auto LayerName = std::string(Layer["name"].GetString());
    auto Data = Layer["data"].GetArray();
    auto &M = Map.get(LayerName);

    auto It = M.begin();
    for (auto &Elem : Data) {
      *It = Elem.GetInt();
      ++It;
    }
  }

  return Map;
}

struct TileInfos {
  std::map<int, std::string> Entities;
  std::map<int, Tile> Tiles;
};

TileInfos loadTileInfos(const std::filesystem::path &TileInfoMap) {
  auto [DocStr, Doc] = loadJSON(TileInfoMap, nullptr);
  auto TileInfosJson = Doc.GetArray();

  TileInfos Infos;
  unsigned TileIdx = 1;
  for (auto &JsonInfo : TileInfosJson) {
    auto Key = std::string(JsonInfo["key"].GetString());
    auto TileType = std::string(JsonInfo["tile_type"].GetString());
    if (TileType == "entity") {
      Infos.Entities[TileIdx] = Key;
    } else {
      Infos.Tiles[TileIdx] = parseTile(JsonInfo["tile"]);
    }
    TileIdx++;
  }

  Infos.Tiles[0] = Level::EmptyTile;
  Infos.Entities[0] = "";

  return Infos;
}

} // namespace

std::shared_ptr<Level>
TiledMapLevelGenerator::createNewLevel(int LevelId) const {
  auto TiledMap = loadTiledMap(Cfg.TiledMapFile);
  auto TileInfos = loadTileInfos(Cfg.TiledIdMapFile);

  auto NewLevel = std::make_shared<Level>(LevelId, TiledMap.getSize());

  static const std::array<std::size_t, 4> LayerIndices = {
      Level::LayerGroundIdx, Level::LayerGroundDecoIdx, Level::LayerWallsIdx,
      Level::LayerWallsDecoIdx};
  for (const auto &Idx : LayerIndices) {
    const auto &Name = Level::LayerNames.at(Idx);
    auto &LvlM = NewLevel->Map.get(Name);
    auto &TiledM = TiledMap.get(Name);

    LvlM.forEach([&TiledM, &TileInfos](auto Pos, auto &Tile) {
      auto TileId = TiledM.getTile(Pos);
      auto It = TileInfos.Tiles.find(TileId);
      if (It == TileInfos.Tiles.end()) {
        std::stringstream SS;
        SS << "Could not find tile for tile Id: " << TileId << " at " << Pos;
        if (auto EtIt = TileInfos.Entities.find(TileId);
            EtIt != TileInfos.Entities.end()) {
          SS << "\nYou are referencing an entity: " << EtIt->second << "";
        }
        throw std::out_of_range(SS.str());
      }
      Tile = TileInfos.Tiles.at(TiledM.getTile(Pos));
    });
  }

  auto &TiledEntitiesMap =
      TiledMap.get(Level::LayerNames.at(Level::LayerEntitiesIdx));

  EntityFactory Factory(NewLevel->Reg, Ctx.EntityDb);
  TiledEntitiesMap.forEach([this, &TileInfos, &Factory,
                            &NewLevel](auto Pos, auto TileId) {
    auto It = TileInfos.Entities.find(TileId);
    if (It == TileInfos.Entities.end()) {
      std::stringstream SS;
      SS << "Could not find entity for tile Id: " << TileId << " at " << Pos;
      if (auto TIt = TileInfos.Tiles.find(TileId);
          TIt != TileInfos.Tiles.end()) {
        SS << "\nYou are referencing a tile: " << TIt->second.kind();
      }
      throw std::out_of_range(SS.str());
    }
    if (It->second.empty()) {
      return;
    }
    spawnAndPlaceEntity(Factory, Pos,
                        Ctx.EntityDb.getEntityTemplateId(It->second),
                        NewLevel->getLevelId());
  });

  return NewLevel;
}

namespace {

DesignedMapLevelGenerator::Config::CharInfo
parseCharInfo(const rapidjson::Value &V) {
  const auto Layer = std::string(V["layer"].GetString());
  const auto Tile = parseTile(V["tile"]);
  return {Tile, Layer};
}

LevelEntityConfig loadLevelEntityConfigFromJSON(const rapidjson::Value &V) {
  LevelEntityConfig Cfg;
  // Load level specific entity configuration
  for (const auto &[Key, Value] : V["entities"].GetObject()) {
    auto KeyStr = std::string(Key.GetString());
    assert(KeyStr.size() == 1);
    Cfg.Entities[KeyStr[0]] = std::string(Value.GetString());
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

TiledMapLevelGenerator::Config
loadTiledMapGeneratorConfig(const std::filesystem::path &BasePath,
                            const rapidjson::Value &Doc) {
  TiledMapLevelGenerator::Config MapCfg;
  auto MapJson = Doc["map"].GetObject();
  MapCfg.TiledMapFile = BasePath / MapJson["tiled_map_file"].GetString();
  MapCfg.TiledIdMapFile = BasePath / MapJson["tiled_id_map_file"].GetString();
  return MapCfg;
}

} // namespace

LevelGeneratorLoader::LevelGeneratorLoader(const GameContext &Ctx,
                                           const std::filesystem::path &DataDir)
    : Ctx(Ctx), DataDir(DataDir) {}

LevelGeneratorLoader::LevelConfig
LevelGeneratorLoader::loadCfg(unsigned Seed,
                              const std::filesystem::path &CfgFile,
                              const std::filesystem::path &DataDir) {
  LevelConfig LvlCfg;

  const auto BasePath = CfgFile.parent_path();
  const auto SchemaDir = DataDir / "schemas";
  const auto SchemaFile = SchemaDir / "level_config_schema.json";
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
  if (MapCfgType == "tiled") {
    return loadTiledMapGeneratorConfig(BasePath, Doc);
  }

  throw std::out_of_range("Invalid map type: " + MapCfgType);
}

std::shared_ptr<LevelGenerator>
LevelGeneratorLoader::create(unsigned Seed, const LevelConfig &Cfg) {
  if (auto *GenCfg = std::get_if<GeneratedMapLevelGenerator::Config>(&Cfg)) {
    return std::make_shared<GeneratedMapLevelGenerator>(Ctx, DataDir, *GenCfg);
  }
  if (auto *DesCfg = std::get_if<DesignedMapLevelGenerator::Config>(&Cfg)) {
    return std::make_shared<DesignedMapLevelGenerator>(Ctx, DataDir, *DesCfg);
  }
  if (auto *EmptyCfg = std::get_if<EmptyLevelGenerator::Config>(&Cfg)) {
    return std::make_shared<EmptyLevelGenerator>(Ctx, DataDir, *EmptyCfg);
  }
  if (auto *CompCfg = std::get_if<CompositeMultiLevelGenerator::Config>(&Cfg)) {
    auto CompGen = std::make_shared<CompositeMultiLevelGenerator>(Ctx, DataDir);
    unsigned LevelGenSeed = Seed;
    for (const auto &[LevelEndIdx, LevelCfg] : CompCfg->Levels) {
      CompGen->addGenerator(load(LevelGenSeed, LevelCfg), LevelEndIdx);
      LevelGenSeed++;
    }
    return CompGen;
  }
  if (auto *TiledCfg = std::get_if<TiledMapLevelGenerator::Config>(&Cfg)) {
    return std::make_shared<TiledMapLevelGenerator>(Ctx, DataDir, *TiledCfg);
  }
  throw std::out_of_range("Invalid map type");
}

std::shared_ptr<LevelGenerator>
LevelGeneratorLoader::load(unsigned Seed,
                           const std::filesystem::path &CfgFile) {
  auto Cfg = loadCfg(Seed, CfgFile, DataDir);
  return create(Seed, Cfg);
}

} // namespace rogue