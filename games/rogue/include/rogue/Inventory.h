#ifndef ROGUE_INVENTORY_H
#define ROGUE_INVENTORY_H

#include <entt/entt.hpp>
#include <rogue/Item.h>
#include <vector>

namespace rogue {

class Inventory {
public:
  const Item &getItem(std::size_t ItemIdx) const { return Items.at(ItemIdx); }
  const std::vector<Item> &getItems() const { return Items; }

  void addItem(const Item &It);
  Item takeItem(std::size_t ItemIdx);
  Item takeItem(std::size_t ItemIdx, unsigned Count);

  std::size_t size() const { return Items.size(); }
  bool empty() const;

private:
  std::vector<Item> Items;
};

} // namespace rogue

#endif // #ifndef ROGUE_INVENTORY_H