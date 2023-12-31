#ifndef ROGUE_ITEM_PROTOTYPE_H
#define ROGUE_ITEM_PROTOTYPE_H

#include <entt/entt.hpp>
#include <memory>
#include <rogue/EffectInfo.h>
#include <rogue/ItemType.h>
#include <vector>

namespace rogue {

/// Describes attributes of an item that are not instance-specific
class ItemPrototype {
public:
  static bool canApply(ItemType Type, CapabilityFlags Flags);

public:
  /// Creates an item prototype with the given attributes
  /// \param ItemId Unique identifier for the item
  /// \param Name Name of the item
  /// \param Description Description of the item
  /// \param Type Type of the item
  /// \param MaxStackSize Maximum number of items of this type that can stack
  /// \param Effects Effects that the item has
  ItemPrototype(int ItemId, std::string Name, std::string Description,
                ItemType Type, int MaxStackSize,
                std::vector<EffectInfo> Effects);

  EffectAttributes getAttributes() const;
  EffectAttributes getAttributes(CapabilityFlags Flags) const;

  bool hasEffect(CapabilityFlags Flags, bool AllowNull = false,
                 bool AllowRemove = false) const;

  CapabilityFlags getCapabilityFlags() const;
  bool checkCapabilityFlags(CapabilityFlags Flags) const;

  bool canApplyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg, CapabilityFlags Flags) const;
  void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
               entt::registry &Reg, CapabilityFlags Flags) const;
  bool canRemoveFrom(const entt::entity &SrcEt, const entt::entity &DstEt,
                     entt::registry &Reg, CapabilityFlags Flags) const;
  void removeFrom(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg, CapabilityFlags Flags) const;

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