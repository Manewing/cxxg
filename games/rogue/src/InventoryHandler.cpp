#include <rogue/Components/Combat.h>
#include <rogue/Components/Entity.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Context.h>
#include <rogue/CraftingHandler.h>
#include <rogue/Equipment.h>
#include <rogue/Event.h>
#include <rogue/Inventory.h>
#include <rogue/InventoryHandler.h>

namespace rogue {

namespace {
std::string getNameOrNone(const entt::registry &Reg, entt::entity Entity) {
  if (Reg.any_of<NameComp>(Entity)) {
    return Reg.get<NameComp>(Entity).Name;
  }
  return "<none>";
}
} // namespace

InventoryHandler::InventoryHandler(entt::entity Entity, entt::registry &Reg,
                                   const CraftingHandler &Crafter)
    : Entity(Entity), Reg(Reg), Crafter(Crafter) {
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
                     getNameOrNone(Reg, Entity));
    }
    Inv->addItem(*EquipItOrNone);
    return true;
  }

  if (IsPlayer) {
    publish(PlayerInfoMessageEvent()
            << "Can not unequip " + Equip->getSlot(Type).It->getName() +
                   " from " + getNameOrNone(Reg, Entity));
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
                                              getNameOrNone(Reg, Entity));
    }
    return false;
  }

  if (IsPlayer) {
    publish(PlayerInfoMessageEvent() << "Equip " + ItCopy.getName() + " on " +
                                            getNameOrNone(Reg, Entity));
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
            << getNameOrNone(Reg, Entity) << " dropped " + It.getName());
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
                                              getNameOrNone(Reg, Entity));
    }
    return true;
  }

  if (IsPlayer) {
    publish(PlayerInfoMessageEvent()
            << "Cannot dismantle " + Inv->getItem(InvItemIdx).getName() +
                   " for " + getNameOrNone(Reg, Entity));
  }

  return false;
}

bool InventoryHandler::tryUseItem(std::size_t InvItemIdx) {
  return tryUseItemOnTarget(InvItemIdx, Entity);
}

bool InventoryHandler::tryUseItemOnTarget(std::size_t InvItemIdx,
                                          entt::entity TargetEt) {
  // Nothing can be used if there is no inventory
  if (!Inv || !Reg.valid(TargetEt)) {
    return false;
  }

  auto ItOrNone =
      Inv->applyItemTo(InvItemIdx, CapabilityFlags::UseOn, TargetEt, Reg);
  if (ItOrNone) {
    if (Entity != TargetEt) {
      // FIXME check if it is actually an attack
      Reg.get_or_emplace<CombatAttackComp>(Entity).Target = TargetEt;
      Reg.get_or_emplace<CombatTargetComp>(TargetEt).Attacker = Entity;
    }

    // FIXME add information on result of use
    if (IsPlayer && Reg.any_of<NameComp>(TargetEt)) {
      publish(PlayerInfoMessageEvent() << "Used " + ItOrNone->getName() +
                                              " on " +
                                              getNameOrNone(Reg, TargetEt));
    }
    return true;
  }

  if (IsPlayer && Reg.any_of<NameComp>(TargetEt)) {
    publish(PlayerInfoMessageEvent()
            << "Can not use " + Inv->getItem(InvItemIdx).getName() + " on " +
                   getNameOrNone(Reg, TargetEt));
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

bool InventoryHandler::tryCraftItems() {
  // Nothing can be crafted if there is no inventory
  if (!Inv) {
    return false;
  }

  // Try crafting all items in the inventory
  auto NewItemsOrNone = Crafter.tryCraft(Inv->getItems());

  // Add all new items to the inventory
  if (NewItemsOrNone) {
    Inv->setItems(*NewItemsOrNone);
    return true;
  }

  return false;
}

} // namespace rogue