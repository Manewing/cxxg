#include "UIController.h"
#include "History.h"
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

HistoryUIController::HistoryUIController(const History &Hist,
                                         unsigned NumHistoryRows)
    : Hist(Hist), NumHistoryRows(NumHistoryRows) {
  auto NumMsgs = Hist.getMessages().size();
  if (NumMsgs > NumHistoryRows) {
    Offset = Hist.getMessages().size() - NumHistoryRows;
  }
}

bool HistoryUIController::handleInput(int Char) {
  switch (Char) {
  case cxxg::utils::KEY_DOWN:
    if (!Hist.getMessages().empty() && Offset < Hist.getMessages().size() - 2) {
      Offset++;
    }
    break;
  case cxxg::utils::KEY_UP:
    if (Offset > 0) {
      Offset--;
    }
    break;
  case 'h':
    return false;
  default:
    break;
  }
  return true;
}

std::string_view HistoryUIController::getInteractMsg() const {
  return "[^/v] Navigate";
}

void HistoryUIController::draw(cxxg::Screen &Scr) const {
  const int PosX = 0, PosY = 2;
  const unsigned NumHistoryRows = 18;
  std::string_view Header = "History";

  // Draw header
  const int HdrOffset = (Scr.getSize().X - Header.size()) / 2 - 1;
  Scr[PosY][PosX] << "+" << std::string(Scr.getSize().X - 2, '-') << "+";
  Scr[PosY][PosX + HdrOffset] << "[" << Header << "]";

  const auto &Msgs = Hist.getMessages();
  for (unsigned int Idx = 0; Idx < NumHistoryRows; Idx++) {
    const int LinePos = PosY + Idx + 1;
    const unsigned MsgPos = Idx + Offset;

    if (MsgPos >= Msgs.size()) {
      Scr[LinePos][PosX] << std::string(Scr.getSize().X, ' ');
      continue;
    }

    if (Idx == 0 && Offset != 0) {
      Scr[LinePos][PosX] << std::string(Scr.getSize().X, ' ');
      Scr[LinePos][PosX + Scr.getSize().X / 2] = '^';
      continue;
    }

    if (Idx == NumHistoryRows - 1 && MsgPos < Msgs.size() - 1) {
      Scr[LinePos][PosX] << std::string(Scr.getSize().X, ' ');
      Scr[LinePos][PosX + Scr.getSize().X / 2] = 'v';
      continue;
    }

    Scr[LinePos] = Msgs.at(MsgPos);
  }

  // Draw footer
  Scr[PosY + NumHistoryRows + 1][PosX]
      << "+" << std::string(Scr.getSize().X - 2, '-') << "+";
}

UIController::UIController(cxxg::Screen &Scr) : Scr(Scr) {}

void UIController::draw(int LevelIdx, int Health,
                        std::string_view InteractStr) {
  auto ScrSize = Scr.getSize();

  // Draw UI overlay
  Scr[0][0] << "Rogue v0.0 [FLOOR]: " << (LevelIdx + 1)
            << " [HEALTH]: " << Health;

  if (ActiveWidget) {
    ActiveWidget->draw(Scr);
    InteractStr = ActiveWidget->getInteractMsg();
  }

  if (!InteractStr.empty()) {
    Scr[ScrSize.Y - 2][ScrSize.X / 2 - InteractStr.size() / 2] << InteractStr;
  }
}

bool UIController::isUIActive() const { return ActiveWidget != nullptr; }

void UIController::handleInput(int Char) {
  if (!ActiveWidget) {
    return;
  }
  if (!ActiveWidget->handleInput(Char)) {
    ActiveWidget = nullptr;
  }
}

void UIController::setInventoryUI(Inventory &Inv) {
  ActiveWidget.reset(new InventoryUIController(Inv));
}

void UIController::setHistoryUI(History &Hist) {
  ActiveWidget.reset(new HistoryUIController(Hist));
}