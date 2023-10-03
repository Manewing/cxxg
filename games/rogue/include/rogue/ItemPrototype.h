#ifndef ROGUE_ITEM_PROTOTYPE_H
#define ROGUE_ITEM_PROTOTYPE_H

#include <entt/entt.hpp>
#include <memory>
#include <rogue/ItemType.h>
#include <vector>

namespace rogue {

class ItemPrototype {
public:
  static bool canApply(ItemType Type, CapabilityFlags Flags);

public:
  ItemPrototype(int ItemId, std::string Name, std::string Description,
                ItemType Type, int MaxStackSize,
                std::vector<EffectInfo> Effects);

  // TODO add:
  // attack melee
  // attack ranged
  // defense
  // craft

  CapabilityFlags getCapabilityFlags() const;

  bool canApplyTo(const entt::entity &Entity, entt::registry &Reg,
                  CapabilityFlags Flags) const;
  void applyTo(const entt::entity &Entity, entt::registry &Reg,
               CapabilityFlags Flags) const;
  bool canRemoveFrom(const entt::entity &Entity, entt::registry &Reg,
                     CapabilityFlags Flags) const;
  void removeFrom(const entt::entity &Entity, entt::registry &Reg,
                  CapabilityFlags Flags) const;

public:
  int ItemId;
  std::string Name;
  std::string Description;
  ItemType Type = ItemType::None;
  int MaxStackSize = 1;

  std::vector<EffectInfo> Effects;
};

} // namespace rogue

#endif // ROGUE_ITEM_PROTOTYPE_H