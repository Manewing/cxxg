#include <cxxg/Utils.h>
#include <iomanip>
#include <rogue/Components/Items.h>
#include <rogue/Inventory.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/Inventory.h>
#include <rogue/UI/ListSelect.h>

namespace rogue::ui {

InventoryControllerBase::InventoryControllerBase(Inventory &Inv,
                                                 entt::entity Entity,
                                                 entt::registry &Reg,
                                                 const std::string &Header)
    : BaseRect({2, 2}, {30, 18}), Inv(Inv), Entity(Entity), Reg(Reg) {
  List = std::make_shared<ListSelect>(Pos, Size);
  Decorated = std::make_shared<Frame>(List, Pos, Size, Header);
  updateElements();
}

void InventoryControllerBase::setPos(cxxg::types::Position P) {
  BaseRect::setPos(P);
  Decorated->setPos(Pos);
}

bool InventoryControllerBase::handleInput(int Char) {
  switch (Char) {
  case 'i':
    return false;
  default:
    return List->handleInput(Char);
  }
  return true;
}

void InventoryControllerBase::draw(cxxg::Screen &Scr) const {
  updateElements();
  BaseRect::draw(Scr);
  Decorated->draw(Scr);
}

void InventoryControllerBase::updateElements() const {
  std::vector<std::string> Elements;
  Elements.reserve(Inv.getItems().size());
  for (const auto &Item : Inv.getItems()) {
    std::stringstream SS;
    SS << std::setw(3) << Item.StackSize << "x " << Item.getName();
    Elements.push_back(SS.str());
  }
  auto PrevIdx = List->getSelectedElement();
  List->setElements(Elements);
  List->selectElement(PrevIdx);
}

InventoryController::InventoryController(Inventory &Inv, entt::entity Entity,
                                         entt::registry &Reg)
    : InventoryControllerBase(Inv, Entity, Reg, "Inventory") {}

bool InventoryController::handleInput(int Char) {
  if (Inv.empty()) {
    return InventoryControllerBase::handleInput(Char);
  }

  switch (Char) {
  case Controls::Equip.Char: {
    auto Equip = Reg.try_get<EquipmentComp>(Entity);
    if (!Equip) {
      // FIXME message
      break;
    }
    const auto &It = Inv.getItem(List->getSelectedElement());

    if (Equip->Equip.isEquipped(It.getType())) {
      auto EquippedIt = Equip->Equip.unequip(It.getType());
      Inv.addItem(EquippedIt);
      // FIXME message
    }

    if (!It.canApplyTo(Entity, Reg, CapabilityFlags::EquipOn)) {
      // FIXME message
      break;
    }
    It.applyTo(Entity, Reg, CapabilityFlags::EquipOn);
    Equip->Equip.equip(Inv.takeItem(List->getSelectedElement(), /*Count=*/1));
    updateElements();
  } break;
  case Controls::Unequip.Char:
    if (!Inv.getItem(List->getSelectedElement())
             .canApplyTo(Entity, Reg, CapabilityFlags::UseOn)) {
      // FIXME message
      break;
    }
    Inv.takeItem(List->getSelectedElement(), /*Count=*/1)
        .applyTo(Entity, Reg, CapabilityFlags::UseOn);
    updateElements();
    break;
  case Controls::Drop.Char:
    // Inv.dropItem(List.getSelectedElement());
    break;
  default:
    return InventoryControllerBase::handleInput(Char);
  }
  return true;
}

std::string InventoryController::getInteractMsg() const {
  if (Inv.empty()) {
    return "";
  }

  const auto &SelectedItem = Inv.getItem(List->getSelectedElement());
  std::vector<KeyOption> Options = {Controls::Drop};
  if ((SelectedItem.getType() & ItemType::EquipmentMask) != ItemType::None) {
    Options.push_back(Controls::Equip);
  }
  if ((SelectedItem.getType() & ItemType::Crafting) != ItemType::None) {
    Options.push_back(Controls::Craft);
  }
  if ((SelectedItem.getType() & ItemType::Consumable) != ItemType::None) {
    Options.push_back(Controls::Use);
  }
  return KeyOption::getInteractMsg(Options);
}

LootController::LootController(Inventory &Inv, entt::entity Entity,
                               entt::registry &Reg)
    : InventoryControllerBase(Inv, Entity, Reg, "Loot") {}

bool LootController::handleInput(int Char) {
  switch (Char) {
  case 'e': {
    if (Inv.empty()) {
      break;
    }
    auto &EtInv = Reg.get<InventoryComp>(Entity).Inv;
    EtInv.addItem(Inv.takeItem(List->getSelectedElement()));
    updateElements();
  } break;
  default:
    return InventoryControllerBase::handleInput(Char);
  }
  return !Inv.empty();
}

std::string LootController::getInteractMsg() const {
  if (Inv.empty()) {
    return "";
  }
  return "[E] Take";
}

} // namespace rogue::ui