#include <rogue/Components/Entity.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Equipment.h>
#include <rogue/Event.h>
#include <rogue/Inventory.h>
#include <rogue/InventoryHandler.h>

namespace rogue {

InventoryHandler::InventoryHandler(entt::entity Entity, entt::registry &Reg)
    : Entity(Entity), Reg(Reg) {
  refresh();
}

void InventoryHandler::refresh() {
  if (auto *InvComp = Reg.try_get<InventoryComp>(Entity)) {
    Inv = &InvComp->Inv;
  }
  if (auto *EquipComp = Reg.try_get<EquipmentComp>(Entity)) {
    Equip = &EquipComp->Equip;
  }
  IsPlayer = Reg.any_of<PlayerComp>(Entity);
}

bool InventoryHandler::tryUnequip(ItemType Type) {
  // Nothing can be unequipped if there is no equipment or inventory
  if (!Inv || !Equip) {
    return false;
  }

  // Check if there is an item equipped for the given type
  if (!Equip->isEquipped(Type)) {
    return false;
  }

  // Try unequip any already equipped item first
  if (auto EquipItOrNone = Equip->tryUnequip(Type, Entity, Reg)) {
    if (IsPlayer) {
      publish(PlayerInfoMessageEvent()
              << "Unequip " + Equip->getSlot(Type).It->getName() + " from " +
                     Reg.get<NameComp>(Entity).Name);
    }
    Inv->addItem(*EquipItOrNone);
    return true;
  }

  if (IsPlayer) {
    publish(PlayerInfoMessageEvent()
            << "Can not unequip " + Equip->getSlot(Type).It->getName() +
                   " from " + Reg.get<NameComp>(Entity).Name);
  }

  return false;
}

bool InventoryHandler::tryEquipItem(std::size_t InvItemIdx) {
  // Nothing can be equipped if there is no equipment or inventory
  if (!Inv || !Equip) {
    return false;
  }
  // Copy item we may change the inventory and invalidate the reference
  const auto ItCopy = Inv->getItem(InvItemIdx);

  // Try unequip any already equipped item first
  tryUnequip(ItCopy.getType());

  // Try equipping the item on the entity, this may not work because it does
  // not allow the item to be equipped or a slot is already occupied the item
  // can not be unequipped from that slot
  if (!Equip->canEquip(ItCopy, Entity, Reg)) {
    if (IsPlayer) {
      publish(PlayerInfoMessageEvent() << "Can not equip " + ItCopy.getName() +
                                              " on " +
                                              Reg.get<NameComp>(Entity).Name);
    }
    return false;
  }

  if (IsPlayer) {
    publish(PlayerInfoMessageEvent() << "Equip " + ItCopy.getName() + " on " +
                                            Reg.get<NameComp>(Entity).Name);
  }

  Equip->equip(Inv->takeItem(InvItemIdx, /*Count=*/1), Entity, Reg);

  return true;
}

bool InventoryHandler::tryDropItem(std::size_t InvItemIdx) {
  // Nothing can be dropped if there is no inventory
  if (!Inv) {
    return false;
  }

  // Get position component can not drop item if entity has no position
  auto *PC = Reg.try_get<PositionComp>(Entity);
  if (!PC) {
    return false;
  }

  // Take item and create drop entity
  auto It = Inv->takeItem(InvItemIdx);
  if (IsPlayer) {
    publish(PlayerInfoMessageEvent()
            << Reg.get<NameComp>(Entity).Name << " dropped " + It.getName());
  }
  Inventory DropInv;
  DropInv.addItem(std::move(It));
  createDropEntity(Reg, PC->Pos, DropInv);

  return true;
}

bool InventoryHandler::tryDismantleItem(std::size_t InvItemIdx) {
  // Nothing can be dismantled if there is no inventory
  if (!Inv) {
    return false;
  }
  auto ItOrNone =
      Inv->applyItemTo(InvItemIdx, CapabilityFlags::Dismantle, Entity, Reg);
  if (ItOrNone) {
    // FIXME add information on result of dismantle
    if (IsPlayer) {
      publish(PlayerInfoMessageEvent() << "Dismantled " + ItOrNone->getName() +
                                              " for " +
                                              Reg.get<NameComp>(Entity).Name);
    }
    return true;
  }

  if (IsPlayer) {
    publish(PlayerInfoMessageEvent()
            << "Cannot dismantle " + Inv->getItem(InvItemIdx).getName() +
                   " for " + Reg.get<NameComp>(Entity).Name);
  }

  return false;
}

bool InventoryHandler::tryUseItem(std::size_t InvItemIdx) {
  // Nothing can be used if there is no inventory
  if (!Inv) {
    return false;
  }
  auto ItOrNone =
      Inv->applyItemTo(InvItemIdx, CapabilityFlags::UseOn, Entity, Reg);
  if (ItOrNone) {
    // FIXME add information on result of use
    if (IsPlayer) {
      publish(PlayerInfoMessageEvent() << "Used " + ItOrNone->getName() +
                                              " on " +
                                              Reg.get<NameComp>(Entity).Name);
    }
    return true;
  }

  if (IsPlayer) {
    publish(PlayerInfoMessageEvent()
            << "Can not use " + Inv->getItem(InvItemIdx).getName() + " on " +
                   Reg.get<NameComp>(Entity).Name);
  }

  return false;
}

void InventoryHandler::autoEquipItems() {
  // Nothing can be auto equipped if there is no equipment or inventory
  if (!Inv || !Equip) {
    return;
  }

  // Try equipping all items in the inventory
  for (std::size_t Idx = 0; Idx < Inv->size(); Idx++) {
    const auto &It = Inv->getItem(Idx);
    if (Equip->canEquip(It, Entity, Reg)) {
      tryEquipItem(Idx);
      Idx -= 1;
    }
  }
}

} // namespace rogue