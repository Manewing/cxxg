#ifndef ROGUE_ITEM_TYPE_H
#define ROGUE_ITEM_TYPE_H

#include <memory>
#include <string>
#include <ymir/Enum.hpp>

namespace rogue {

enum class ItemType {
  None = 0x0,

  // Equipment types
  Ring = 0x1,
  Amulet = 0x2,
  Helmet = 0x4,
  ChestPlate = 0x8,
  Pants = 0x10,
  Boots = 0x20,
  Weapon = 0x40,
  Shield = 0x80,
  Ranged = 0x100,
  EquipmentMask = 0xfff,

  /// Items that can be used as a basis for crafting a new item
  CraftingBase = 0x1000,

  /// Consumable items can be used to apply an effect to an entity
  Consumable = 0x2000,

  Quest = 0x4000,
  Crafting = 0x8000,
  GeneralMask = 0xf000,

  // mask for all items
  AnyMask = 0xfffffff
};

ItemType getItemType(const std::string &Type);
std::string getItemTypeLabel(ItemType Type);

enum class CapabilityFlags {
  None = 0x0,
  UseOn = 0x1,
  EquipOn = 0x2,
  UnequipFrom = 0x4,

  // Mask all equipment
  Equipment = 0x6,

  // Dismantle item
  Dismantle = 0x8,

  // Ranged, indicates that the item can be used from a distance on an entity
  // different than the entity using the item
  Ranged = 0x10,

  // Adjacent, indicates that the item can be used from an adjacent tile on an
  // entity different than the entity using the item
  Adjacent = 0x20,

  // Self, indicates that the item can be used on the entity using the item
  Self = 0x40,
};

CapabilityFlags getCapabilityFlag(const std::string &CapabilityFlagStr);
const char *getCapabilityFlagLabel(CapabilityFlags Flags);

class ItemEffect;
struct EffectInfo {
  CapabilityFlags Flags;
  std::shared_ptr<ItemEffect> Effect;
};

} // namespace rogue

YMIR_BITFIELD_ENUM(rogue::ItemType);
YMIR_BITFIELD_ENUM(rogue::CapabilityFlags);

#endif // #ifndef ROGUE_ITEM_TYPE_H