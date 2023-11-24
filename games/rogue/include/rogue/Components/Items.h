#ifndef ROGUE_COMPONENTS_ITEMS_H
#define ROGUE_COMPONENTS_ITEMS_H

#include <rogue/Equipment.h>
#include <rogue/Inventory.h>

namespace rogue {

struct EquipmentComp {
  Equipment Equip;
};

struct InventoryComp {
  Inventory Inv;
  bool IsPersistent = true;
  bool Looted = false;
};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_ITEMS_H