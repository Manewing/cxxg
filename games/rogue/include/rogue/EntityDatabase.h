#ifndef ROGUE_ENTITY_DATABASE_H
#define ROGUE_ENTITY_DATABASE_H

#include <entt/entt.hpp>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace rogue {
class ItemDatabase;
}

namespace rogue {

struct EntityTemplateId {
  inline explicit EntityTemplateId(int Id) : Id(Id) {}
  inline operator int() const { return Id; }
  int Id = -1;
};

class EntityAssembler {
public:
  virtual ~EntityAssembler() = default;
  virtual void assemble(entt::registry &Reg, entt::entity Entity) const = 0;
};

struct EntityTemplateInfo {
  std::string Name = "<unimpl. name>";
  std::string DisplayName = "<unimpl. display name>";
  std::string Description = "<unimpl. desc>";
  EntityTemplateId Id = EntityTemplateId(-1);

  std::map<std::string, std::shared_ptr<EntityAssembler>> Assemblers;
};

class EntityDatabase {
public:
  class EntityAssemblerCache {
  public:
    void add(const std::string &Name,
             const std::shared_ptr<EntityAssembler> &Assembler);

    template <typename T, typename... Args>
    void add(const std::string &Name, Args &&...args) {
      add(Name, std::make_shared<T>(std::forward<Args>(args)...));
    }

    const std::shared_ptr<EntityAssembler> &get(const std::string &Name) const;

  private:
    std::map<std::string, std::shared_ptr<EntityAssembler>> Assemblers;
  };

public:
  static EntityDatabase load(const std::filesystem::path &EntityDbConfig);

public:
  inline auto empty() const { return EntityTemplateInfos.empty(); }
  inline auto size() const { return EntityTemplateInfos.size(); }

  EntityTemplateId
  getEntityTemplateId(const std::string &EntityTemplateName) const;
  void addEntityTemplate(EntityTemplateInfo EntityTemplateInfo);

  const EntityTemplateInfo &getEntityTemplate(EntityTemplateId Id) const;

private:
  /// Map entity name to it's Id
  std::map<std::string, EntityTemplateId> EntityTemplateIdsByName;

  /// Map entity template by it's Id
  std::vector<EntityTemplateInfo> EntityTemplateInfos;
};

class EntityFactory {
public:
  EntityFactory(entt::registry &Reg, const EntityDatabase &EntityDb,
                const ItemDatabase &ItemDb);

  entt::entity createEntity(EntityTemplateId Id) const;

private:
  entt::registry &Reg;
  const EntityDatabase &EntityDb;
  const ItemDatabase &ItemDb;
};

} // namespace rogue

#endif // ROGUE_ENTITY_DATABASE_H