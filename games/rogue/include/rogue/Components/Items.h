#ifndef ROGUE_COMPONENTS_ITEMS_H
#define ROGUE_COMPONENTS_ITEMS_H

#include <rogue/Inventory.h>
#include <rogue/Equipment.h>

namespace rogue {

struct EquipmentComp {
  Equipment Equip;
};

struct InventoryComp {
  Inventory Inv;
};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_ITEMS_H