#include "UIController.h"
#include "History.h"
#include "Inventory.h"
#include "UIListSelect.h"
#include <cxxg/Screen.h>
#include <cxxg/Utils.h>

#include "Components/Items.h"

InventoryUIControllerBase::InventoryUIControllerBase(Inventory &Inv,
                                                     entt::entity Entity,
                                                     entt::registry &Reg,
                                                     const std::string &Header)
    : Inv(Inv), Entity(Entity), Reg(Reg), ListUI(Header, 30, 18) {
  updateElements();
}

bool InventoryUIControllerBase::handleInput(int Char) {
  switch (Char) {
  case cxxg::utils::KEY_DOWN:
    ListUI.selectNext();
    break;
  case cxxg::utils::KEY_UP:
    ListUI.selectPrev();
    break;
  case 'i':
    return false;
  default:
    break;
  }
  return true;
}

void InventoryUIControllerBase::draw(cxxg::Screen &Scr) const {
  ListUI.draw(Scr);
}

void InventoryUIControllerBase::updateElements() {
  std::vector<std::string> Elements;
  Elements.reserve(Inv.getItems().size());
  for (const auto &Item : Inv.getItems()) {
    std::stringstream SS;
    SS << Item.StackSize << "x " << Item.getProto().Name;
    Elements.push_back(SS.str());
  }
  auto PrevIdx = ListUI.getSelectedElement();
  ListUI.setElements(Elements);
  ListUI.selectElement(PrevIdx);
}

InventoryUIController::InventoryUIController(Inventory &Inv,
                                             entt::entity Entity,
                                             entt::registry &Reg)
    : InventoryUIControllerBase(Inv, Entity, Reg, "Inventory") {}

bool InventoryUIController::handleInput(int Char) {
  switch (Char) {
  case 'e':
    if (Inv.empty() ||
        !Inv.canUseItem(Entity, Reg, ListUI.getSelectedElement())) {
      break;
    }
    Inv.useItem(Entity, Reg, ListUI.getSelectedElement(), 1);
    updateElements();
    break;
  case 'd':
    // Inv.dropItem(ListUI.getSelectedElement());
    break;
  default:
    return InventoryUIControllerBase::handleInput(Char);
  }
  return true;
}

std::string_view InventoryUIController::getInteractMsg() const {
  if (Inv.empty()) {
    return "[Empty]";
  }
  // FIXME item may have multiple options...
  const auto &SelectedItem = Inv.getItem(ListUI.getSelectedElement());
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

LootUIController::LootUIController(Inventory &Inv, entt::entity Entity,
                                   entt::registry &Reg)
    : InventoryUIControllerBase(Inv, Entity, Reg, "Loot") {}

bool LootUIController::handleInput(int Char) {
  switch (Char) {
  case 'e': {
    if (Inv.empty()) {
      break;
    }
    auto &EtInv = Reg.get<InventoryComp>(Entity).Inv;
    EtInv.addItem(Inv.takeItem(ListUI.getSelectedElement()));
    updateElements();
  } break;
  default:
    return InventoryUIControllerBase::handleInput(Char);
  }
  return true;
}

std::string_view LootUIController::getInteractMsg() const {
  if (Inv.empty()) {
    return "[Empty]";
  }
  return "[E] Take";
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
  const unsigned Start =
      std::min(Msgs.size(), static_cast<std::size_t>(Offset + 1));
  const unsigned End =
      std::min(Msgs.size(), static_cast<std::size_t>(Offset + NumHistoryRows));
  std::string Footer = std::to_string(Start) + "-" + std::to_string(End);

  Scr[PosY + NumHistoryRows + 1][PosX]
      << "+" << std::string(Scr.getSize().X - 2, '-') << "+";
  const int FtrOffset = (Scr.getSize().X - Footer.size()) / 2 - 1;
  Scr[PosY + NumHistoryRows + 1][PosX + FtrOffset] << "[" << Footer << "]";
}

UIController::UIController(cxxg::Screen &Scr) : Scr(Scr) {}

void UIController::draw(int LevelIdx, int Health,
                        std::string_view InteractStr) {
  auto ScrSize = Scr.getSize();

  // Draw UI overlay
  Scr[0][0] << cxxg::types::Color::NONE.underline() << "Rogue v0.0 [FLOOR]: "
            << (LevelIdx + 1)
            << " [HEALTH]: "
            << Health;

  if (ActiveWidget) {
    ActiveWidget->draw(Scr);
    InteractStr = ActiveWidget->getInteractMsg();
  }

  if (!InteractStr.empty()) {
    Scr[2][ScrSize.X / 2 - InteractStr.size() / 2] << InteractStr;
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

void UIController::setInventoryUI(Inventory &Inv, entt::entity Entity,
                                  entt::registry &Reg) {
  ActiveWidget.reset(new InventoryUIController(Inv, Entity, Reg));
}

void UIController::setLootUI(Inventory &Inv, entt::entity Entity,
                             entt::registry &Reg) {
  ActiveWidget.reset(new LootUIController(Inv, Entity, Reg));
}

void UIController::setHistoryUI(History &Hist) {
  ActiveWidget.reset(new HistoryUIController(Hist));
}