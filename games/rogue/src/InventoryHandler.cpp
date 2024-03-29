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
#include <rogue/ItemEffect.h>

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
  auto ItOrNone = Inv->applyItemTo(InvItemIdx, CapabilityFlags::Dismantle,
                                   Entity, Entity, Reg);
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
  const auto &It = Inv->getItem(InvItemIdx);
  if (!It.hasEffect(CapabilityFlags::UseOn)) {
    if (IsPlayer) {
      publish(PlayerInfoMessageEvent()
              << It.getName() << " does not have a use effect");
    }
    return false;
  }

  const auto FlagsSelf = CapabilityFlags::UseOn | CapabilityFlags::Self;
  const auto FlagsAdj = CapabilityFlags::UseOn | CapabilityFlags::Adjacent;
  const auto FlagsRanged = CapabilityFlags::UseOn | CapabilityFlags::Ranged;

  if (Entity != TargetEt && !It.canApplyTo(Entity, TargetEt, Reg, FlagsAdj) &&
      !It.canApplyTo(Entity, TargetEt, Reg, FlagsRanged)) {
    if (IsPlayer && Reg.any_of<NameComp>(TargetEt)) {
      std::stringstream SS;
      SS << "Can not use " << It.getName() << " on "
         << getNameOrNone(Reg, TargetEt);
      publish(PlayerInfoMessageEvent() << SS.str());
    }
    return false;
  }

  if (!It.canApplyTo(Entity, TargetEt, Reg, CapabilityFlags::UseOn)) {
    if (IsPlayer && Reg.any_of<NameComp>(TargetEt)) {
      std::stringstream SS;
      SS << "Can not use " << It.getName() << " on "
         << getNameOrNone(Reg, TargetEt);
      publish(PlayerInfoMessageEvent() << SS.str());
    }
    return false;
  }

  It.applyTo(Entity, Entity, Reg, FlagsSelf);
  It.applyTo(Entity, TargetEt, Reg, FlagsAdj);
  It.applyTo(Entity, TargetEt, Reg, FlagsRanged);

  if (IsPlayer && Reg.any_of<NameComp>(TargetEt)) {
    std::stringstream SS;
    SS << "Used " << It.getName() << " on " << getNameOrNone(Reg, TargetEt);
    publish(PlayerInfoMessageEvent() << SS.str());
  }

  Inv->takeItem(InvItemIdx, /*Count=*/1);

  return true;
}

bool InventoryHandler::tryUseSkill(ItemType SlotType) {
  return tryUseSkillOnTarget(SlotType, Entity);
}

void InventoryHandler::handleCanNotUseSkill(entt::entity TargetEt,
                                            Item const &It) {
  if (IsPlayer) {
    std::stringstream SS;
    SS << "Can not use skill ";
    for (const auto &Info : It.getAllEffects()) {
      if (Info.Attributes.Flags.is(CapabilityFlags::Skill)) {
        SS << Info.Effect->getName() << " ";
      }
    }
    SS << "of " << It.getName() << " on " << getNameOrNone(Reg, TargetEt);
    publish(PlayerInfoMessageEvent() << SS.str());
  }
}

bool InventoryHandler::tryUseSkillOnTarget(ItemType SlotType,
                                           entt::entity TargetEt) {
  // Nothing can be used if there is no equipment
  if (!Equip || !Reg.valid(TargetEt)) {
    return false;
  }

  const auto &Slot = Equip->getSlot(SlotType);
  if (!Slot.It) {
    return false;
  }
  const auto &It = *Slot.It;

  const auto FlagsSelf = CapabilityFlags::Skill | CapabilityFlags::Self;
  const auto FlagsAdj = CapabilityFlags::Skill | CapabilityFlags::Adjacent;
  const auto FlagsRanged = CapabilityFlags::Skill | CapabilityFlags::Ranged;

  if (!It.hasEffect(CapabilityFlags::Skill)) {
    std::stringstream SS;
    SS << It.getName() << " does not have a skill";
    publish(PlayerInfoMessageEvent() << SS.str());
    return false;
  }

  // If there is another target check ranged flags
  if (Entity != TargetEt && !It.canApplyTo(Entity, TargetEt, Reg, FlagsAdj) &&
      !It.canApplyTo(Entity, TargetEt, Reg, FlagsRanged)) {
    handleCanNotUseSkill(TargetEt, It);
    return false;
  }

  // Check overall use of skill
  if (!It.canApplyTo(Entity, TargetEt, Reg, CapabilityFlags::Skill)) {
    handleCanNotUseSkill(TargetEt, It);
    return false;
  }

  It.applyTo(Entity, Entity, Reg, FlagsSelf);
  It.applyTo(Entity, TargetEt, Reg, FlagsAdj);
  It.applyTo(Entity, TargetEt, Reg, FlagsRanged);

  if (IsPlayer && Reg.any_of<NameComp>(TargetEt)) {
    std::stringstream SS;
    SS << "Used skill ";
    for (const auto &Info : It.getAllEffects()) {
      if (Info.Attributes.Flags.is(CapabilityFlags::Skill)) {
        SS << Info.Effect->getName() << " ";
      }
    }
    SS << "of " << It.getName() << " on " << getNameOrNone(Reg, TargetEt);
    publish(PlayerInfoMessageEvent() << SS.str());
  }

  return true;
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

bool InventoryHandler::tryCraftItems(entt::entity SrcEt) {
  // Nothing can be crafted if there is no inventory
  if (!Inv) {
    return false;
  }
  const auto &Items = Inv->getItems();

  if (SrcEt == entt::null) {
    SrcEt = Entity;
  }
  bool IsRecipe = false;
  if (auto *PC = Reg.try_get<PlayerComp>(SrcEt)) {
    if (auto Result = Crafter.getCraftingRecipeResultOrNone(Items)) {
      PC->KnownRecipes.insert(Result->RecipeId);
      IsRecipe = true;
    }
  }

  // Try crafting all items in the inventory
  auto NewItemsOrNone = Crafter.tryCraft(Items);

  // Add all new items to the inventory
  if (NewItemsOrNone) {
    if (SrcEt != Entity && Reg.valid(SrcEt) &&
        Reg.all_of<InventoryComp>(SrcEt)) {
      auto &SrcInv = Reg.get<InventoryComp>(SrcEt).Inv;
      for (auto &It : *NewItemsOrNone) {
        SrcInv.addItem(std::move(It));
      }
      Inv->clear();
    } else {
      Inv->setItems(*NewItemsOrNone);
    }

    if (Reg.any_of<PlayerComp>(SrcEt) || IsPlayer) {
      std::stringstream SS;
      SS << (IsRecipe ? "Crafted " : "Enhanced ");
      const char *Pred = "";
      for (const auto &It : *NewItemsOrNone) {
        SS << Pred << It.getName();
        Pred = ", ";
      }
      publish(PlayerInfoMessageEvent() << SS.str());
    }

    return true;
  }

  if (Reg.any_of<PlayerComp>(SrcEt) || IsPlayer) {
    publish(PlayerInfoMessageEvent() << "Crafting failed");
  }

  return false;
}

bool InventoryHandler::canCraft(const CraftingRecipe &Recipe) const {
  // Nothing can be crafted if there is no inventory
  if (!Inv) {
    return false;
  }

  std::map<int, unsigned> ItemCounts;
  for (const auto &ItId : Recipe.getRequiredItems()) {
    ItemCounts[ItId] += 1;
  }

  for (const auto [ItId, Count] : ItemCounts) {
    if (!Inv->hasItem(ItId, Count)) {
      return false;
    }
  }

  return true;
}

bool InventoryHandler::tryCraft(const CraftingRecipe &Recipe) {
  // Nothing can be crafted if there is no inventory
  if (!Inv) {
    return false;
  }

  if (!canCraft(Recipe)) {
    return false;
  }

  std::vector<Item> CraftItems;
  for (const auto &ItId : Recipe.getRequiredItems()) {
    if (auto IdxOrNone = Inv->getItemIndexForId(ItId)) {
      CraftItems.push_back(Inv->takeItem(*IdxOrNone, 1));
    }
  }

  auto NewItemsOrNone = Crafter.tryCraft(CraftItems);
  if (!NewItemsOrNone) {
    for (auto &It : CraftItems) {
      Inv->addItem(std::move(It));
    }
    return false;
  }

  for (auto &It : *NewItemsOrNone) {
    Inv->addItem(std::move(It));
  }

  return true;
}

} // namespace rogue