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

  EntityAssembler &get(const std::string &Name);
  const EntityAssembler &get(const std::string &Name) const;
  EntityAssembler *getOrNull(const std::string &Name);
  const EntityAssembler *getOrNull(const std::string &Name) const;

  template <typename T> T &get(const std::string &Name) {
    return const_cast<EntityAssembler &>(
        const_cast<const EntityTemplateInfo *>(this)->get<T>(Name));
  }

  template <typename T> const T &get(std::string Name) const {
    const auto &Asm = get(Name);
    if (const auto *Cast = dynamic_cast<const T *>(&Asm)) {
      return *Cast;
    }
    throw std::runtime_error("Failed to cast entity assembler: " +
                             std::string(Name) + " to " + typeid(T).name());
  }

  template <typename T> T *getOrNull(const std::string &Name) {
    return const_cast<EntityAssembler *>(
        const_cast<const EntityTemplateInfo *>(this)->getOrNull<T>(Name));
  }

  template <typename T> const T *getOrNull(const std::string &Name) const {
    auto *Asm = getOrNull(Name);
    if (!Asm) {
      return nullptr;
    }
    if (auto *Cast = dynamic_cast<const T *>(Asm)) {
      return *Cast;
    }
    return nullptr;
  }
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