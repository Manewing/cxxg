#include <rogue/EntityAssemblers.h>
#include <rogue/EntityDatabase.h>
#include <rogue/JSON.h>

namespace rogue {

void EntityDatabase::EntityAssemblerCache::add(
    const std::string &Name,
    const std::shared_ptr<EntityAssembler> &Assembler) {
  Assemblers.emplace(Name, Assembler);
}

const std::shared_ptr<EntityAssembler> &
EntityDatabase::EntityAssemblerCache::get(const std::string &Name) const {
  auto It = Assemblers.find(Name);
  if (It == Assemblers.end()) {
    throw std::out_of_range("Unknown entity assembler name: " + Name);
  }
  return It->second;
}

namespace {

EntityDatabase::EntityAssemblerCache getDefaultEntityAssemblerCache() {
  EntityDatabase::EntityAssemblerCache Cache;
  // Keep sorted
  Cache.add<AttackAICompAssembler>("attack_ai");
  Cache.add<CollisionCompAssembler>("collision");
  Cache.add<HealthCompAssembler>("health");
  Cache.add<PlayerCompAssembler>("player");
  Cache.add<PositionCompAssembler>("position");
  Cache.add<TileCompAssembler>("tile");
  Cache.add<VisibleCompAssembler>("visible");
  Cache.add<WanderAICompAssembler>("wander_ai");

  // TODO make configurable
  Cache.add<LineOfSightCompAssembler>("line_of_sight");
  Cache.add<AgilityCompAssembler>("agility");

  return Cache;
}

} // namespace

EntityDatabase
EntityDatabase::load(const std::filesystem::path &EntityDbConfig) {
  EntityDatabase Db;

  const auto SchemaPath =
      EntityDbConfig.parent_path() / "entity_db_schema.json";
  auto [DocStr, Doc] = loadJSON(EntityDbConfig, &SchemaPath);

  auto DefaultAssemblers = getDefaultEntityAssemblerCache();

  auto Entities = Doc["entities"].GetArray();
  for (const auto &EntityJson : Entities) {
    EntityTemplateInfo EntityInfo;
    EntityInfo.Name = std::string(EntityJson["name"].GetString());
    EntityInfo.DisplayName =
        std::string(EntityJson["display_name"].GetString());
    EntityInfo.Description = std::string(EntityJson["description"].GetString());
    EntityInfo.Id = EntityTemplateId(Db.EntityTemplateInfos.size());

    auto Assemblers = EntityJson["assemblers"].GetArray();
    for (const auto &AssemblerJson : Assemblers) {
      const auto AssemblerName = std::string(AssemblerJson["name"].GetString());
      const auto AssemblerType = std::string(AssemblerJson["type"].GetString());

      if (AssemblerType == "tile") {
        EntityInfo.Assemblers.emplace(AssemblerName,
                                      std::make_shared<TileCompAssembler>());
      } else if (AssemblerType == "collision") {
        EntityInfo.Assemblers.emplace(
            AssemblerName, std::make_shared<CollisionCompAssembler>());
      } else if (AssemblerType == "visible") {
        EntityInfo.Assemblers.emplace(AssemblerName,
                                      std::make_shared<VisibleCompAssembler>());
      } else {
        throw std::runtime_error("Unknown assembler type: " + AssemblerType);
      }
    }

    Db.addEntityTemplate(std::move(EntityInfo));
  }

  return Db;
}

EntityFactory::EntityFactory(entt::registry &Reg,
                             const EntityDatabase &EntityDb,
                             const ItemDatabase &ItemDb)
    : Reg(Reg), EntityDb(EntityDb), ItemDb(ItemDb) {}

entt::entity EntityFactory::createEntity(EntityTemplateId Id) const {
  auto Entity = Reg.create();
  auto &EtTmpl = EntityDb.getEntityTemplate(Id);

  Reg.emplace<NameComp>(Entity, EtTmpl.DisplayName);

  for (auto &[AsmNm, Assembler] : EtTmpl.Assemblers) {
    Assembler->assemble(Reg, Entity);
  }
  // TODO fill inventory
  (void)ItemDb;

  return Entity;
}

} // namespace rogue