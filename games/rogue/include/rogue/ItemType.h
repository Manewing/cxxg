#ifndef ROGUE_ITEM_TYPE_H
#define ROGUE_ITEM_TYPE_H

#include <ymir/Enum.hpp>
#include <string>

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
  OffHand = 0x80,
  EquipmentMask = 0xff,

  // general item types
  Generic = 0x100,
  Consumable = 0x200,
  Quest = 0x400,
  Crafting = 0x800,
  GeneralMask = 0xf00,

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
};

CapabilityFlags
getCapabilityFlag(const std::string &CapabilityFlagStr);
const char *getCapabilityFlagLabel(CapabilityFlags Flags);

} // namespace rogue

YMIR_BITFIELD_ENUM(rogue::ItemType);
YMIR_BITFIELD_ENUM(rogue::CapabilityFlags);

#endif // #ifndef ROGUE_ITEM_TYPE_H