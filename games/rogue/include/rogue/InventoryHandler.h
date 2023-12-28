#ifndef ROGUE_INVENTORY_HANDLER_H
#define ROGUE_INVENTORY_HANDLER_H

#include <cstddef>
#include <entt/entt.hpp>
#include <rogue/EventHub.h>
#include <rogue/ItemType.h>

namespace rogue {
class Inventory;
class Equipment;
class CraftingRecipe;
class CraftingHandler;
} // namespace rogue

namespace rogue {

class InventoryHandler : public EventHubConnector {
public:
  explicit InventoryHandler(entt::entity Entity, entt::registry &Reg,
                            const CraftingHandler &Crafter);

  /// Updates the inventory handler with the latest inventory and equipment
  void refresh();

  /// Tries to unequip the item with the given item type
  bool tryUnequip(ItemType Type);

  /// Tries to equip the item at the given inventory index to the entity
  /// @param InvItemIdx The index of the item in the inventory
  /// @return True if the item was equipped, false otherwise
  bool tryEquipItem(std::size_t InvItemIdx);

  /// Drop item from inventory
  /// @param InvItemIdx The index of the item in the inventory
  /// @return True if the item was dropped, false otherwise
  bool tryDropItem(std::size_t InvItemIdx);

  /// Try dismantle item from inventory
  /// @param InvItemIdx The index of the item in the inventory
  /// @return True if the item was dismantled, false otherwise
  bool tryDismantleItem(std::size_t InvItemIdx);

  /// Try using item from inventory
  /// @param InvItemIdx The index of the item in the inventory
  /// @return True if the item was used, false otherwise
  bool tryUseItem(std::size_t InvItemIdx);

  /// Try using item from inventory on target entity
  /// @param InvItemIdx The index of the item in the inventory
  /// @param TargetEt The target entity to use the item on
  /// @return True if the item was used, false otherwise
  bool tryUseItemOnTarget(std::size_t InvItemIdx, entt::entity TargetEt);

  /// Auto equip items from inventory
  void autoEquipItems();

  /// Tries to craft the items in the inventory
  bool tryCraftItems();

  /// Checks if the inventory has all items to craft the recipe
  bool canCraft(const CraftingRecipe &Recipe) const;

  /// Tries to craft the recipe from the inventory
  bool tryCraft(const CraftingRecipe &Recipe);

private:
  entt::entity Entity;
  entt::registry &Reg;
  const CraftingHandler &Crafter;

  Inventory *Inv = nullptr;
  Equipment *Equip = nullptr;

  // FIXME we can get rid of this when changing to general events for
  // equip/unequip, etc.
  bool IsPlayer = false;
};

} // namespace rogue

#endif // #ifndef ROGUE_INVENTORY_HANDLER_H