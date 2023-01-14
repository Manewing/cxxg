#include <cxxg/Utils.h>
#include <rogue/Components/Items.h>
#include <rogue/Inventory.h>
#include <rogue/UI/Inventory.h>

namespace rogue::ui {

InventoryControllerBase::InventoryControllerBase(Inventory &Inv,
                                                 entt::entity Entity,
                                                 entt::registry &Reg,
                                                 const std::string &Header)
    : Inv(Inv), Entity(Entity), Reg(Reg), List(Header, 30, 18) {
  updateElements();
}

bool InventoryControllerBase::handleInput(int Char) {
  switch (Char) {
  case cxxg::utils::KEY_DOWN:
    List.selectNext();
    break;
  case cxxg::utils::KEY_UP:
    List.selectPrev();
    break;
  case 'i':
    return false;
  default:
    break;
  }
  return true;
}

void InventoryControllerBase::draw(cxxg::Screen &Scr) const { List.draw(Scr); }

void InventoryControllerBase::updateElements() {
  std::vector<std::string> Elements;
  Elements.reserve(Inv.getItems().size());
  for (const auto &Item : Inv.getItems()) {
    std::stringstream SS;
    SS << Item.StackSize << "x " << Item.getName();
    Elements.push_back(SS.str());
  }
  auto PrevIdx = List.getSelectedElement();
  List.setElements(Elements);
  List.selectElement(PrevIdx);
}

InventoryController::InventoryController(Inventory &Inv, entt::entity Entity,
                                         entt::registry &Reg)
    : InventoryControllerBase(Inv, Entity, Reg, "Inventory") {}

bool InventoryController::handleInput(int Char) {
  if (Inv.empty()) {
    return true;
  }

  switch (Char) {
  case 'u':
    if (Inv.canUseItem(Entity, Reg, List.getSelectedElement())) {
      break;
    }
    Inv.useItem(Entity, Reg, List.getSelectedElement(), 1);
    updateElements();
    break;
  case 'd':
    // Inv.dropItem(List.getSelectedElement());
    break;
  default:
    return InventoryControllerBase::handleInput(Char);
  }
  return true;
}

std::string_view InventoryController::getInteractMsg() const {
  if (Inv.empty()) {
    return "[Empty]";
  }
  // FIXME item may have multiple options...
  const auto &SelectedItem = Inv.getItem(List.getSelectedElement());
  if ((SelectedItem.getType() & ItemType::EquipmentMask) != ItemType::None) {
    return "[E] Equip";
  }
  if ((SelectedItem.getType() & ItemType::Crafting) != ItemType::None) {
    return "[C] Craft";
  }
  if ((SelectedItem.getType() & ItemType::Consumable) != ItemType::None) {
    return "[U] Use";
  }
  return "[D] Drop";
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
    EtInv.addItem(Inv.takeItem(List.getSelectedElement()));
    updateElements();
  } break;
  default:
    return InventoryControllerBase::handleInput(Char);
  }
  return true;
}

std::string_view LootController::getInteractMsg() const {
  if (Inv.empty()) {
    return "[Empty]";
  }
  return "[E] Take";
}

} // namespace rogue::ui