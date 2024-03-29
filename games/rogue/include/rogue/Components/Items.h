#ifndef ROGUE_COMPONENTS_ITEMS_H
#define ROGUE_COMPONENTS_ITEMS_H

#include <rogue/Components/Visual.h>
#include <rogue/Equipment.h>
#include <rogue/Inventory.h>

namespace rogue {

struct EquipmentComp {
  Equipment Equip;
};

struct DropEquipmentComp {};

struct InventoryComp {
  Inventory Inv;
};

struct LootInteractComp {
  bool IsLooted = false;
  bool IsPersistent = true;
  Tile DefaultTile;
  Tile LootedTile;
  std::string LootName;
};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_ITEMS_H