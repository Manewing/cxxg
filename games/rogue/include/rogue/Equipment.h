#ifndef ROGUE_EQUIPMENT_H
#define ROGUE_EQUIPMENT_H

#include <array>
#include <optional>
#include <rogue/Item.h>
#include <rogue/ItemType.h>

namespace rogue {

struct EquipmentSlot {
  const ItemType BaseTypeFilter = ItemType::None;
  std::optional<Item> It = std::nullopt;

  inline bool empty() const { return It == std::nullopt; }

  void equip(Item Item);
  Item unequip();
};

class Equipment {
public:
  EquipmentSlot Ring = {ItemType::Ring};
  EquipmentSlot Amulet = {ItemType::Amulet};
  EquipmentSlot Helmet = {ItemType::Helmet};
  EquipmentSlot ChestPlate = {ItemType::ChestPlate};
  EquipmentSlot Pants = {ItemType::Pants};
  EquipmentSlot Boots = {ItemType::Boots};
  EquipmentSlot Weapon = {ItemType::Weapon};
  EquipmentSlot OffHand = {ItemType::Shield | ItemType::Ranged};

  Equipment() = default;

  inline std::array<EquipmentSlot *, 8> all() {
    return {&Ring,  &Amulet, &Helmet, &ChestPlate,
            &Pants, &Boots,  &Weapon, &OffHand};
  }

  EquipmentSlot &getSlot(ItemType It);
  const EquipmentSlot &getSlot(ItemType It) const;

  bool isEquipped(ItemType Type) const;
  const Item *getEquipped(ItemType Type) const;
  bool canEquip(const Item &It, entt::entity Entity, entt::registry &Reg) const;
  bool canUnequip(ItemType Type, entt::entity Entity,
                  entt::registry &Reg) const;
  void equip(Item Item, entt::entity Entity, entt::registry &Reg);
  Item unequip(ItemType Type, entt::entity Entity, entt::registry &Reg);

  std::optional<Item> tryUnequip(ItemType Type, entt::entity Entity,
                                 entt::registry &Reg);
};

} // namespace rogue

#endif // #ifndef ROGUE_EQUIPMENT_H