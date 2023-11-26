#include <rogue/EntityAssemblers.h>
#include <rogue/EntityDatabase.h>
#include <rogue/JSON.h>
#include <rogue/JSONHelpers.h>

namespace rogue {

void EntityTemplateInfo::from(const EntityTemplateInfo &Parent) {
  Assemblers = Parent.Assemblers;
  if (!DisplayName) {
    DisplayName = Parent.DisplayName;
  }
  if (!Description) {
    Description = Parent.Description;
  }
}

const char *EntityTemplateInfo::getDisplayName() const {
  if (DisplayName) {
    return DisplayName->c_str();
  }
  return "<unimpl. display name>";
}

const char *EntityTemplateInfo::getDescription() const {
  if (Description) {
    return Description->c_str();
  }
  return "<unimpl. description>";
}

void EntityAssemblerCache::add(
    const std::string &Name,
    const std::shared_ptr<EntityAssembler> &Assembler) {
  Assemblers.emplace(Name, Assembler);
}

const std::shared_ptr<EntityAssembler> &
EntityAssemblerCache::get(const std::string &Name) const {
  auto It = Assemblers.find(Name);
  if (It == Assemblers.end()) {
    throw std::out_of_range("Unknown entity assembler name: " + Name);
  }
  return It->second;
}

const std::shared_ptr<EntityAssembler> *
EntityAssemblerCache::getOrNull(const std::string &Name) const {
  auto It = Assemblers.find(Name);
  if (It == Assemblers.end()) {
    return nullptr;
  }
  return &It->second;
}

namespace {

EntityAssemblerCache getDefaultEntityAssemblerCache() {
  EntityAssemblerCache Cache;
  // Keep sorted
  Cache.add<AttackAICompAssembler>("attack_ai");
  Cache.add<AutoEquipAssembler>("auto_equip");
  Cache.add<BlockLOSCompAssembler>("block_los");
  Cache.add<CollisionCompAssembler>("collision");
  Cache.add<EquipmentCompAssembler>("equipment");
  Cache.add<HealthCompAssembler>("health");
  Cache.add<PlayerCompAssembler>("player");
  Cache.add<PositionCompAssembler>("position");
  Cache.add<VisibleCompAssembler>("visible");
  Cache.add<WanderAICompAssembler>("wander_ai");

  // TODO make configurable
  Cache.add<LineOfSightCompAssembler>("line_of_sight");
  Cache.add<AgilityCompAssembler>("agility");
  Cache.add<HealerInteractableCompAssembler>("healer");
  Cache.add<ShopAssembler>("shop");
  Cache.add<WorkbenchAssembler>("work_bench");

  return Cache;
}

std::shared_ptr<TileCompAssembler>
makeTileCompAssembler(ItemDatabase &, const rapidjson::Value &Json) {
  Tile T = parseTile(Json);
  return std::make_shared<TileCompAssembler>(T);
}

std::shared_ptr<FactionCompAssembler>
makeFactionCompAssembler(ItemDatabase &, const rapidjson::Value &Json) {
  FactionKind F = getFaction(Json.GetString());
  return std::make_shared<FactionCompAssembler>(F);
}

std::shared_ptr<RaceCompAssembler>
makeRaceCompAssembler(ItemDatabase &, const rapidjson::Value &Json) {
  RaceKind R = getRace(Json.GetString());
  return std::make_shared<RaceCompAssembler>(R);
}

std::shared_ptr<InventoryCompAssembler>
makeInventoryCompAssembler(ItemDatabase &ItemDb, const rapidjson::Value &Json) {
  const auto &JsonObj = Json.GetObject();
  std::string LootTable = JsonObj["loot_table"].GetString();
  bool IsPersistent = JsonObj["is_persistent"].GetBool();
  bool IsLooted = JsonObj["is_looted"].GetBool();
  return std::make_shared<InventoryCompAssembler>(ItemDb, LootTable,
                                                  IsPersistent, IsLooted);
}

std::shared_ptr<ChestInteractableCompAssembler>
makeChestInteractableCompAssembler(ItemDatabase &,
                                   const rapidjson::Value &Json) {
  const auto &JsonObj = Json.GetObject();
  Tile T = parseTile(JsonObj["looted_tile"]);
  return std::make_shared<ChestInteractableCompAssembler>(T);
}

std::shared_ptr<DoorCompAssembler>
makeDoorCompAssembler(ItemDatabase &ItemDb, const rapidjson::Value &Json) {
  const auto &JsonObj = Json.GetObject();
  Tile OpenTile = parseTile(JsonObj["open_tile"]);
  Tile ClosedTile = parseTile(JsonObj["closed_tile"]);
  bool IsOpen = JsonObj["is_open"].GetBool();
  std::optional<int> KeyId; // TODO
  (void)ItemDb;
  return std::make_shared<DoorCompAssembler>(IsOpen, OpenTile, ClosedTile,
                                             KeyId);
}

std::shared_ptr<WorldEntryInteractableCompAssembler>
makeWorldEntryAssembler(ItemDatabase &, const rapidjson::Value &Json) {
  const auto &JsonObj = Json.GetObject();
  std::string LevelName = JsonObj["level_name"].GetString();
  return std::make_shared<WorldEntryInteractableCompAssembler>(LevelName);
}

std::shared_ptr<StatsCompAssembler>
makeStatsCompAssembler(ItemDatabase&, const rapidjson::Value &Json) {
  auto SP = parseStatPoints(Json);
  return std::make_shared<StatsCompAssembler>(SP);
}

using EntityAssemblerFactories =
    std::map<std::string, std::function<std::shared_ptr<EntityAssembler>(
                              ItemDatabase &, const rapidjson::Value &)>>;

const auto &getEntityAssemblerFactories() {
  static const EntityAssemblerFactories Factories = {
      {"chest", makeChestInteractableCompAssembler},
      {"door", makeDoorCompAssembler},
      {"faction", makeFactionCompAssembler},
      {"inventory", makeInventoryCompAssembler},
      {"race", makeRaceCompAssembler},
      {"stats", makeStatsCompAssembler},
      {"tile", makeTileCompAssembler},
      {"world_entry", makeWorldEntryAssembler},
  };
  return Factories;
}

void setupDefaultAssemblers(EntityTemplateInfo &Info,
                            const EntityAssemblerCache &DefaultAssemblers,
                            const std::string &AsmName, bool IsPresent) {
  if (IsPresent) {
    const auto &AsmPtr = DefaultAssemblers.get(AsmName);
    // Overwrite any present assembler
    Info.Assemblers[AsmName] = AsmPtr;
    return;
  }

  // Remove assembler if it exists
  if (auto It = Info.Assemblers.find(AsmName); It != Info.Assemblers.end()) {
    Info.Assemblers.erase(It);
  }
}

EntityTemplateInfo getEntityTemplateInfoFromJson(const rapidjson::Value &Json) {
  EntityTemplateInfo Info;
  Info.Name = std::string(Json["name"].GetString());
  if (Json.HasMember("display_name")) {
    Info.DisplayName = std::string(Json["display_name"].GetString());
  }
  if (Json.HasMember("description")) {
    Info.Description = std::string(Json["description"].GetString());
  }
  return Info;
}

void createEntityTemplates(EntityDatabase &Db, ItemDatabase &ItemDb,
                           const EntityAssemblerCache &DefaultAssemblers,
                           const EntityAssemblerFactories &AssemblerFactories,
                           const rapidjson::Value &Json) {

  for (const auto &EntityJson : Json.GetArray()) {
    auto Info = getEntityTemplateInfoFromJson(EntityJson);

    // Handle inheritance
    if (EntityJson.HasMember("from")) {
      auto From = std::string(EntityJson["from"].GetString());
      Info.from(Db.getEntityTemplate(Db.getEntityTemplateId(From)));
    }

    auto AssemblersJson = EntityJson["assemblers"].GetObject();
    for (const auto &[Name, Data] : AssemblersJson) {
      auto AsmName = std::string(Name.GetString());

      // Look up assembler in default assembler cache
      if (Data.IsBool()) {
        setupDefaultAssemblers(Info, DefaultAssemblers, AsmName,
                               Data.GetBool());
        continue;
      } else if (DefaultAssemblers.getOrNull(AsmName)) {
        throw std::runtime_error("Expected boolean for default assembler: " +
                                 AsmName);
      }

      // Look up assembler in assembler factories
      auto It = AssemblerFactories.find(AsmName);
      if (It == AssemblerFactories.end()) {
        throw std::runtime_error("Unknown entity assembler: " + AsmName);
      }
      Info.Assemblers[AsmName] = It->second(ItemDb, Data);
    }
    Db.addEntityTemplate(std::move(Info));
  }
}

} // namespace

EntityDatabase
EntityDatabase::load(ItemDatabase &ItemDb,
                     const std::filesystem::path &EntityDbConfig) {
  EntityDatabase Db;

  //  const auto SchemaPath =
  //      EntityDbConfig.parent_path() / "entity_db_schema.json";
  auto [DocStr, Doc] = loadJSON(EntityDbConfig, nullptr);

  // Create all entity templates
  const auto DefaultAssemblers = getDefaultEntityAssemblerCache();
  const auto &AssemblerFactories = getEntityAssemblerFactories();
  createEntityTemplates(Db, ItemDb, DefaultAssemblers, AssemblerFactories,
                        Doc["entity_templates"]);

  return Db;
}

bool EntityDatabase::hasEntityTemplate(const std::string &EntityTemplateName) const {
  return EntityTemplateIdsByName.count(EntityTemplateName) > 0;
}

EntityTemplateId EntityDatabase::getEntityTemplateId(
    const std::string &EntityTemplateName) const {
  auto It = EntityTemplateIdsByName.find(EntityTemplateName);
  if (It == EntityTemplateIdsByName.end()) {
    throw std::out_of_range("Unknown entity template name: " +
                            EntityTemplateName);
  }
  return It->second;
}

void EntityDatabase::addEntityTemplate(EntityTemplateInfo EntityTemplateInfo) {
  EntityTemplateInfo.Id = EntityTemplateId(EntityTemplateInfos.size());
  EntityTemplateIdsByName.emplace(EntityTemplateInfo.Name,
                                  EntityTemplateInfo.Id);
  EntityTemplateInfos.emplace_back(std::move(EntityTemplateInfo));
}

EntityTemplateInfo &EntityDatabase::getEntityTemplate(EntityTemplateId Id) {
  return const_cast<EntityTemplateInfo &>(
      static_cast<const EntityDatabase *>(this)->getEntityTemplate(Id));
}

const EntityTemplateInfo &
EntityDatabase::getEntityTemplate(EntityTemplateId Id) const {
  if (EntityTemplateInfos.empty() || Id > EntityTemplateInfos.size()) {
    throw std::out_of_range("Unknown entity template id: " +
                            std::to_string(Id));
  }
  return EntityTemplateInfos[Id];
}

EntityFactory::EntityFactory(entt::registry &Reg,
                             const EntityDatabase &EntityDb)
    : Reg(Reg), EntityDb(EntityDb) {}

entt::registry &EntityFactory::getRegistry() { return Reg; }

entt::entity EntityFactory::createEntity(EntityTemplateId Id) const {
  auto Entity = Reg.create();
  auto &EtTmpl = EntityDb.getEntityTemplate(Id);

  Reg.emplace<NameComp>(Entity, EtTmpl.getDisplayName());

  // Assemble entity
  for (auto &[AsmNm, Assembler] : EtTmpl.Assemblers) {
    if (Assembler->isPostProcess()) {
      continue;
    }
    Assembler->assemble(Reg, Entity);
  }

  // Run post-process assemblers
  for (auto &[AsmNm, Assembler] : EtTmpl.Assemblers) {
    if (!Assembler->isPostProcess()) {
      continue;
    }
    Assembler->assemble(Reg, Entity);
  }

  return Entity;
}

} // namespace rogue