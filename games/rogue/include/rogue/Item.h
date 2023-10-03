#ifndef ROGUE_ITEM_H
#define ROGUE_ITEM_H

#include <array>
#include <entt/entt.hpp>
#include <memory>
#include <rogue/ItemType.h>
#include <string>
#include <vector>

namespace rogue {
class ItemPrototype;
}

namespace rogue {

class Item {
public:
  Item(const ItemPrototype &Proto, int StackSize = 1,
       const std::shared_ptr<ItemPrototype> &Specialization = nullptr);
  virtual ~Item() = default;

  std::string getName() const;
  std::string getDescription() const;
  ItemType getType() const;
  int getMaxStackSize() const;
  std::vector<EffectInfo> getAllEffects() const;

  CapabilityFlags getCapabilityFlags() const;

  /// Returns true if other Item has same prototype and specialization
  bool isSameKind(const Item &Other) const;

  bool canApplyTo(const entt::entity &Entity, entt::registry &Reg,
                  CapabilityFlags Flags) const;
  void applyTo(const entt::entity &Entity, entt::registry &Reg,
               CapabilityFlags Flags) const;
  bool canRemoveFrom(const entt::entity &Entity, entt::registry &Reg,
                     CapabilityFlags Flags) const;
  void removeFrom(const entt::entity &Entity, entt::registry &Reg,
                  CapabilityFlags Flags) const;

private:
  const ItemPrototype &getProto() const;

public:
  int StackSize = 1;

private:
  const ItemPrototype *Proto = nullptr;
  std::shared_ptr<const ItemPrototype> Specialization = nullptr;
};

} // namespace rogue

#endif // #ifndef ROGUE_ITEM_H