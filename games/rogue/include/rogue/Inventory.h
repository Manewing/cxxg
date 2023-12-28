#ifndef ROGUE_INVENTORY_H
#define ROGUE_INVENTORY_H

#include <entt/entt.hpp>
#include <optional>
#include <rogue/Item.h>
#include <vector>

namespace rogue {

class Inventory {
public:
  /// Applies the item to the entity if possible
  /// @param It The item to apply
  /// @param Flags The capability flags used for applying
  /// @param Entity The entity to apply the item to
  /// @param Reg The registry the entity belongs to
  /// @return True if the item was applied, false otherwise
  static bool applyItemTo(const Item &It, CapabilityFlags Flags,
                          entt::entity Entity, entt::registry &Reg);

public:
  Inventory() = default;
  Inventory(unsigned MaxStackSize) : MaxStackSize(MaxStackSize) {}

  const Item &getItem(std::size_t ItemIdx) const { return Items.at(ItemIdx); }
  void setItems(std::vector<Item> Items) { this->Items = std::move(Items); }
  const std::vector<Item> &getItems() const { return Items; }

  unsigned getMaxStackSize() const { return MaxStackSize; }

  void addItem(Item It);
  Item takeItem(std::size_t ItemIdx);
  Item takeItem(std::size_t ItemIdx, unsigned Count);

  /// Returns true if the inventory contains an item with the given id and count
  bool hasItem(int Id, unsigned Count = 1) const;

  std::optional<std::size_t> getItemIndexForId(int Id) const;

  std::optional<Item> applyItemTo(std::size_t ItemIdx, CapabilityFlags Flags,
                                  entt::entity Entity, entt::registry &Reg);

  std::size_t size() const;
  bool empty() const;
  void clear();

private:
  std::vector<Item> Items;

  /// A max stack size of zero indicates no limit
  unsigned MaxStackSize = 0;
};

std::ostream &operator<<(std::ostream &OS, const Inventory &Inv);

} // namespace rogue

#endif // #ifndef ROGUE_INVENTORY_H