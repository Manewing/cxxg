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
  const Item &getItem(std::size_t ItemIdx) const { return Items.at(ItemIdx); }
  const std::vector<Item> &getItems() const { return Items; }

  void addItem(Item It);
  Item takeItem(std::size_t ItemIdx);
  Item takeItem(std::size_t ItemIdx, unsigned Count);

  std::optional<std::size_t> getItemIndexForId(int Id) const;

  std::optional<Item> applyItemTo(std::size_t ItemIdx, CapabilityFlags Flags,
                                  entt::entity Entity, entt::registry &Reg);

  std::size_t size() const;
  bool empty() const;

private:
  std::vector<Item> Items;
};

} // namespace rogue

#endif // #ifndef ROGUE_INVENTORY_H