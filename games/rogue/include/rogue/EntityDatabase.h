#ifndef ROGUE_ENTITY_DATABASE_H
#define ROGUE_ENTITY_DATABASE_H

#include <entt/entt.hpp>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace rogue {
class ItemDatabase;
}

namespace rogue {

struct EntityTemplateId {
  inline explicit EntityTemplateId(std::size_t Id) : Id(Id) {}
  inline operator std::size_t() const { return Id; }
  std::size_t Id = -1U;
};

class EntityAssembler {
public:
  virtual ~EntityAssembler() = default;
  virtual bool isPostProcess() const { return false; }
  virtual void assemble(entt::registry &Reg, entt::entity Entity) const = 0;
};

struct EntityTemplateInfo {
  std::string Name = "<unimpl. name>";
  std::optional<std::string> DisplayName;
  std::optional<std::string> Description;
  EntityTemplateId Id = EntityTemplateId(-1);
  std::map<std::string, std::shared_ptr<EntityAssembler>> Assemblers;

  /// Inherit from another entity template
  void from(const EntityTemplateInfo &Parent);

  const char *getDisplayName() const;
  const char *getDescription() const;
};

class EntityAssemblerCache {
public:
  void add(const std::string &Name,
           const std::shared_ptr<EntityAssembler> &Assembler);

  template <typename T, typename... Args>
  void add(const std::string &Name, Args &&...args) {
    add(Name, std::make_shared<T>(std::forward<Args>(args)...));
  }

  const std::shared_ptr<EntityAssembler> *
  getOrNull(const std::string &Name) const;
  const std::shared_ptr<EntityAssembler> &get(const std::string &Name) const;

private:
  std::map<std::string, std::shared_ptr<EntityAssembler>> Assemblers;
};

class EntityDatabase {
public:
  static EntityDatabase load(ItemDatabase &ItemDb,
                             const std::filesystem::path &EntityDbConfig);

public:
  inline auto empty() const { return EntityTemplateInfos.empty(); }
  inline auto size() const { return EntityTemplateInfos.size(); }

  bool hasEntityTemplate(const std::string &EntityTemplateName) const;
  EntityTemplateId
  getEntityTemplateId(const std::string &EntityTemplateName) const;

  void addEntityTemplate(EntityTemplateInfo EntityTemplateInfo);

  EntityTemplateInfo &getEntityTemplate(EntityTemplateId Id);
  const EntityTemplateInfo &getEntityTemplate(EntityTemplateId Id) const;

private:
  /// Map entity name to it's Id
  std::map<std::string, EntityTemplateId> EntityTemplateIdsByName;

  /// Map entity template by it's Id
  std::vector<EntityTemplateInfo> EntityTemplateInfos;
};

class EntityFactory {
public:
  EntityFactory(entt::registry &Reg, const EntityDatabase &EntityDb);

  entt::registry &getRegistry();
  entt::entity createEntity(EntityTemplateId Id) const;

private:
  entt::registry &Reg;
  const EntityDatabase &EntityDb;
};

} // namespace rogue

#endif // ROGUE_ENTITY_DATABASE_H