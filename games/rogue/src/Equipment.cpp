#include <rogue/Equipment.h>

namespace rogue {

void EquipmentSlot::equip(Item I) {
  if ((I.getType() & BaseTypeFilter) == ItemType::None) {
    throw std::runtime_error("Can not equip item: " + I.getName() +
                             +" of type " + getItemTypeLabel(I.getType()) +
                             " (" + std::to_string(int(I.getType())) +
                             ") to slot: " + getItemTypeLabel(BaseTypeFilter) +
                             " (" + std::to_string(int(BaseTypeFilter)) + ")");
  }
  It = std::move(I);
}

Item EquipmentSlot::unequip() {
  assert(It != std::nullopt);
  Item I = std::move(*It);
  It = std::nullopt;
  return I;
}

EquipmentSlot &Equipment::getSlot(ItemType It) {
  return const_cast<EquipmentSlot &>(
      static_cast<const Equipment *>(this)->getSlot(It));
}

const EquipmentSlot &Equipment::getSlot(ItemType It) const {
  switch (It & ItemType::EquipmentMask) {
  case ItemType::Ring:
    return Ring;
  case ItemType::Amulet:
    return Amulet;
  case ItemType::Helmet:
    return Helmet;
  case ItemType::ChestPlate:
    return ChestPlate;
  case ItemType::Pants:
    return Pants;
  case ItemType::Boots:
    return Boots;
  case ItemType::Weapon:
    return Weapon;
  case ItemType::OffHand:
    return OffHand;
  default:
    break;
  }
  assert(false);
  return OffHand;
}

bool Equipment::isEquipped(ItemType Type) const {
  if ((Type & ItemType::EquipmentMask) == ItemType::None) {
    return false;
  }
  return getSlot(Type).It != std::nullopt;
}

bool Equipment::canEquip(const Item &It, entt::entity Entity,
                         entt::registry &Reg) const {
  const auto Type = It.getType();
  if (isEquipped(Type)) {
    return false;
  }
  return It.canApplyTo(Entity, Reg, CapabilityFlags::EquipOn);
}

bool Equipment::canUnequip(ItemType Type, entt::entity Entity,
                           entt::registry &Reg) const {
  if (!isEquipped(Type)) {
    return false;
  }
  return getSlot(Type).It->canRemoveFrom(Entity, Reg,
                                         CapabilityFlags::UnequipFrom);
}

void Equipment::equip(Item It, entt::entity Entity, entt::registry &Reg) {
  assert(canEquip(It, Entity, Reg) && "Can not equip item on entity");
  It.applyTo(Entity, Reg, CapabilityFlags::EquipOn);
  getSlot(It.getType()).equip(std::move(It));
}

Item Equipment::unequip(ItemType Type, entt::entity Entity,
                        entt::registry &Reg) {
  assert(canUnequip(Type, Entity, Reg) &&
         "Can not unequip. Unequip removes item effects from entity");
  auto &Slot = getSlot(Type);
  Slot.It->removeFrom(Entity, Reg, CapabilityFlags::UnequipFrom);
  return Slot.unequip();
}

std::optional<Item> Equipment::tryUnequip(ItemType Type, entt::entity Entity,
                                          entt::registry &Reg) {
  if (!canUnequip(Type, Entity, Reg)) {
    return std::nullopt;
  }
  return unequip(Type, Entity, Reg);
}

} // namespace rogue