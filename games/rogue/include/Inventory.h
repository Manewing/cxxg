#ifndef ROGUE_INVENTORY_H
#define ROGUE_INVENTORY_H

#include "Item.h"
#include <vector>

class Entity;

class Inventory {
public:
  Inventory(Entity &Parent);

  void consumeItem(std::size_t ItemIdx, int NumItems = 1);

  bool empty() const;

  Entity &Parent;
  std::vector<Item> Items;
};

#endif // #ifndef ROGUE_INVENTORY_H