#include "UIController.h"
#include "Inventory.h"
#include "UIListSelect.h"
#include <cxxg/Screen.h>
#include <cxxg/Utils.h>

InventoryUIController::InventoryUIController(Inventory &Inv)
    : Inv(Inv), ListUI("Inventory", 30, 18) {
  updateElements();
}

bool InventoryUIController::handleInput(int Char) {
  switch (Char) {
  case cxxg::utils::KEY_DOWN:
    ListUI.selectNext();
    break;
  case cxxg::utils::KEY_UP:
    ListUI.selectPrev();
    break;
  case 'i':
    return false;
  case 'e':
    if (Inv.empty()) {
      break;
    }
    Inv.consumeItem(ListUI.getSelectedElement(), 1);
    updateElements();
    break;
  case 'd':
    // Inv.dropItem(ListUI.getSelectedElement());
    break;
  default:
    break;
  }
  return true;
}

std::string_view InventoryUIController::getInteractMsg() const {
  if (Inv.empty()) {
    return "[Empty]";
  }
  // FIXME item may have multiple options...
  const auto &SelectedItem = Inv.Items.at(ListUI.getSelectedElement());
  if (SelectedItem.getProto().Type & ItemType::EQUIPMENT_MASK) {
    return "[E] Equip";
  }
  if (SelectedItem.getProto().Type & ItemType::CRAFTING) {
    return "[C] Craft";
  }
  if (SelectedItem.getProto().Type & ItemType::CONSUMABLE) {
    return "[E] Consume";
  }
  return "[D] Drop";
}

void InventoryUIController::draw(cxxg::Screen &Scr) const { ListUI.draw(Scr); }

void InventoryUIController::updateElements() {
  std::vector<std::string> Elements;
  Elements.reserve(Inv.Items.size());
  for (const auto &Item : Inv.Items) {
    std::stringstream SS;
    SS << Item.StackSize << "x " << Item.getProto().Name;
    Elements.push_back(SS.str());
  }
  auto PrevIdx = ListUI.getSelectedElement();
  ListUI.setElements(Elements);
  ListUI.selectElement(PrevIdx);
}

UIController::UIController(cxxg::Screen &Scr) : Scr(Scr) {}

void UIController::draw(int LevelIdx, int Health,
                        std::string_view InteractStr) {
  auto ScrSize = Scr.getSize();

  // Draw UI overlay
  Scr[0][0] << "Rogue v0.0 [FLOOR]: " << (LevelIdx + 1)
            << " [HEALTH]: " << Health;

  if (InventoryUI) {
    InventoryUI->draw(Scr);
    InteractStr = InventoryUI->getInteractMsg();
  }

  if (!InteractStr.empty()) {
    Scr[ScrSize.Y - 2][ScrSize.X / 2 - InteractStr.size() / 2] << InteractStr;
  }
}

bool UIController::isUIActive() const { return InventoryUI != nullptr; }

void UIController::handleInput(int Char) {
  if (!InventoryUI) {
    return;
  }
  if (!InventoryUI->handleInput(Char)) {
    InventoryUI = nullptr;
  }
}

void UIController::setInventoryUI(Inventory &Inv) {
  InventoryUI.reset(new InventoryUIController(Inv));
}