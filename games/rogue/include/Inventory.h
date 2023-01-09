#ifndef ROGUE_INVENTORY_H
#define ROGUE_INVENTORY_H

#include "Item.h"
#include <entt/entt.hpp>
#include <vector>

class Inventory {
public:
  const Item &getItem(std::size_t ItemIdx) const { return Items.at(ItemIdx); }
  const std::vector<Item> &getItems() const { return Items; }

  void addItem(const Item &It);
  Item takeItem(std::size_t ItemIdx);
  bool canUseItem(const entt::entity &Entity, entt::registry &Reg,
                  std::size_t ItemIdx);
  void useItem(const entt::entity &Entity, entt::registry &Reg,
               std::size_t ItemIdx, int NumItems = 1);

  bool empty() const;

private:
  std::vector<Item> Items;
};

#endif // #ifndef ROGUE_INVENTORY_H