#ifndef ROGUE_COMPONENTS_ITEMS_H
#define ROGUE_COMPONENTS_ITEMS_H

#include "Inventory.h"

struct InventoryComp {
  Inventory Inv;
};

struct ChestComp {
  Inventory Inv;
};

struct DropComp {
  Inventory Inv;
};

#endif // #ifndef ROGUE_COMPONENTS_ITEMS_H