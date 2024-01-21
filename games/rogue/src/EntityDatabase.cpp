#include <rogue/EntityAssemblers.h>
#include <rogue/EntityDatabase.h>
#include <rogue/ItemDatabase.h>
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

EntityAssembler &EntityTemplateInfo::get(const std::string &Name) {
  return const_cast<EntityAssembler &>(
      const_cast<const EntityTemplateInfo *>(this)->get(Name));
}

const EntityAssembler &EntityTemplateInfo::get(const std::string &Name) const {
  auto It = Assemblers.find(Name);
  if (It == Assemblers.end()) {
    throw std::out_of_range("Unknown entity assembler: " + std::string(Name));
  }
  return *It->second;
}

EntityAssembler *EntityTemplateInfo::getOrNull(const std::string &Name) {
  return const_cast<EntityAssembler *>(
      const_cast<const EntityTemplateInfo *>(this)->getOrNull(Name));
}

const EntityAssembler *
EntityTemplateInfo::getOrNull(const std::string &Name) const {
  auto It = Assemblers.find(Name);
  if (It == Assemblers.end()) {
    return nullptr;
  }
  return It->second.get();
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
  Cache.add<DropEquipAssembler>("drop_equipment");
  Cache.add<EquipmentCompAssembler>("equipment");
  Cache.add<HealthCompAssembler>("health");
  Cache.add<ManaCompAssembler>("mana");
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
  unsigned MaxStackSize = 0;
  if (JsonObj.HasMember("max_stack_size")) {
    MaxStackSize = JsonObj["max_stack_size"].GetUint();
  }
  return std::make_shared<InventoryCompAssembler>(ItemDb, LootTable,
                                                  MaxStackSize);
}

std::shared_ptr<LootedInteractCompAssembler>
makeLootInteractCompAssembler(ItemDatabase &, const rapidjson::Value &Json) {
  const auto &JsonObj = Json.GetObject();
  std::string InteractText = JsonObj["interact_text"].GetString();
  std::string LootName = JsonObj["loot_name"].GetString();
  bool IsPersistent = JsonObj["is_persistent"].GetBool();
  bool IsLooted = JsonObj["is_looted"].GetBool();
  Tile DefaultTile = parseTile(JsonObj["default_tile"]);
  Tile LootedTile = parseTile(JsonObj["looted_tile"]);
  return std::make_shared<LootedInteractCompAssembler>(
      IsLooted, IsPersistent, DefaultTile, LootedTile, InteractText, LootName);
}

int createNewKey(ItemDatabase &ItemDb) {
  auto KeyTemplateId = ItemDb.getItemId("Key");
  auto KeyTemplate = ItemDb.getItemProto(KeyTemplateId);
  KeyTemplate.ItemId = ItemDb.getNewItemId();
  KeyTemplate.Name = "Key " + std::to_string(KeyTemplate.ItemId);
  ItemDb.addItemProto(KeyTemplate);
  return KeyTemplate.ItemId;
}

std::shared_ptr<DoorCompAssembler>
makeDoorCompAssembler(ItemDatabase &ItemDb, const rapidjson::Value &Json) {
  const auto &JsonObj = Json.GetObject();
  Tile OpenTile = parseTile(JsonObj["open_tile"]);
  Tile ClosedTile = parseTile(JsonObj["closed_tile"]);
  bool IsOpen = JsonObj["is_open"].GetBool();
  std::optional<int> KeyId; // TODO
  if (JsonObj.HasMember("key")) {
    // FIXME allow creating keys on the fly
    (void)createNewKey;
    KeyId = ItemDb.getItemId(JsonObj["key"].GetString());
  }
  return std::make_shared<DoorCompAssembler>(IsOpen, OpenTile, ClosedTile,
                                             KeyId);
}

std::shared_ptr<WorldEntryInteractableCompAssembler>
makeWorldEntryAssembler(ItemDatabase &, const rapidjson::Value &Json) {
  const auto &JsonObj = Json.GetObject();
  std::string LevelName = JsonObj["level_name"].GetString();
  return std::make_shared<WorldEntryInteractableCompAssembler>(LevelName);
}

std::shared_ptr<LevelEntryExitAssembler>
makeLevelEntryExitAssembler(ItemDatabase &, const rapidjson::Value &Json) {
  const auto &JsonObj = Json.GetObject();
  bool IsExit = JsonObj["is_exit"].GetBool();
  int LevelId = JsonObj["level_id"].GetInt();
  return std::make_shared<LevelEntryExitAssembler>(IsExit, LevelId);
}

std::shared_ptr<SpawnEntityPostInteractionAssembler>
makeSpawnEntityPostInteraction(ItemDatabase &, const rapidjson::Value &Json) {
  const auto &JsonObj = Json.GetObject();
  std::string EntityName = JsonObj["entity_name"].GetString();
  double Chance = 0;
  if (JsonObj.HasMember("chance")) {
    Chance = JsonObj["chance"].GetDouble();
  }
  return std::make_shared<SpawnEntityPostInteractionAssembler>(EntityName,
                                                               Chance);
}

std::shared_ptr<StatsCompAssembler>
makeStatsCompAssembler(ItemDatabase &, const rapidjson::Value &Json) {
  auto SP = parseStatPoints(Json);
  return std::make_shared<StatsCompAssembler>(SP);
}

DamageComp parseDamageComp(const rapidjson::Value &Json) {
  DamageComp DC;
  const auto &JsonObj = Json.GetObject();
  DC.PhysDamage = JsonObj["phys_damage"].GetUint();
  DC.MagicDamage = JsonObj["magic_damage"].GetUint();
  if (JsonObj.HasMember("hits")) {
    DC.Hits = JsonObj["hits"].GetUint();
  }
  if (JsonObj.HasMember("ticks")) {
    DC.Ticks = JsonObj["ticks"].GetUint();
  }
  return DC;
}

std::shared_ptr<DamageCompAssembler>
makeDamageCompAssembler(ItemDatabase &, const rapidjson::Value &Json) {
  auto DC = parseDamageComp(Json);
  return std::make_shared<DamageCompAssembler>(DC);
}

using EntityAssemblerFactories =
    std::map<std::string, std::function<std::shared_ptr<EntityAssembler>(
                              ItemDatabase &, const rapidjson::Value &)>>;

const auto &getEntityAssemblerFactories() {
  static const EntityAssemblerFactories Factories = {
      {"door", makeDoorCompAssembler},
      {"faction", makeFactionCompAssembler},
      {"inventory", makeInventoryCompAssembler},
      {"race", makeRaceCompAssembler},
      {"stats", makeStatsCompAssembler},
      {"damage", makeDamageCompAssembler},
      {"tile", makeTileCompAssembler},
      {"world_entry", makeWorldEntryAssembler},
      {"level_entry_exit", makeLevelEntryExitAssembler},
      {"spawn_entity_post_interaction", makeSpawnEntityPostInteraction},
      {"loot_interact", makeLootInteractCompAssembler}};
  return Factories;
}

void setupDefaultAssembler(EntityTemplateInfo &Info,
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
                           rapidjson::Document &Doc,
                           const EntityAssemblerCache &DefaultAssemblers,
                           const EntityAssemblerFactories &AssemblerFactories) {
  auto &Json = Doc["entity_templates"];

  std::map<std::string, rapidjson::Value> EntityAssemblerJsons;
  for (const auto &EntityJson : Json.GetArray()) {
    auto Info = getEntityTemplateInfoFromJson(EntityJson);
    auto &AssemblersJson = EntityJson["assemblers"];
    rapidjson::Value AssemblersMergedJson;
    AssemblersMergedJson.CopyFrom(AssemblersJson, Doc.GetAllocator());

    // Handle inheritance
    if (EntityJson.HasMember("from_template")) {
      auto From = std::string(EntityJson["from_template"].GetString());
      Info.from(Db.getEntityTemplate(Db.getEntityTemplateId(From)));
    }

    for (const auto &[Name, Data] : AssemblersMergedJson.GetObject()) {
      auto AsmName = std::string(Name.GetString());

      // Look up assembler in default assembler cache
      if (Data.IsBool()) {
        setupDefaultAssembler(Info, DefaultAssemblers, AsmName, Data.GetBool());
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

    EntityAssemblerJsons[Info.Name] = std::move(AssemblersMergedJson);
    Db.addEntityTemplate(std::move(Info));
  }
}

} // namespace

EntityDatabase
EntityDatabase::load(ItemDatabase &ItemDb,
                     const std::filesystem::path &EntityDbConfig) {
  EntityDatabase Db;

  const auto SchemaPath =
      EntityDbConfig.parent_path() / "schemas" / "entity_db_schema.json";
  auto [DocStr, Doc] = loadJSON(EntityDbConfig, &SchemaPath);

  // Create all entity templates
  const auto DefaultAssemblers = getDefaultEntityAssemblerCache();
  const auto &AssemblerFactories = getEntityAssemblerFactories();
  createEntityTemplates(Db, ItemDb, Doc, DefaultAssemblers, AssemblerFactories);

  return Db;
}

bool EntityDatabase::hasEntityTemplate(
    const std::string &EntityTemplateName) const {
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

  Reg.emplace<NameComp>(Entity, EtTmpl.getDisplayName(),
                        EtTmpl.getDescription());

  // Assemble entity
  try {
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
  } catch (const std::exception &E) {
    throw std::runtime_error("Failed to assemble entity: " + EtTmpl.Name +
                             " -> " + std::string(E.what()));
  }

  return Entity;
}

} // namespace rogue