#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <rogue/UI/Controller.h>
#include <rogue/UI/History.h>
#include <rogue/UI/Inventory.h>

namespace rogue::ui {

Controller::Controller(cxxg::Screen &Scr) : Scr(Scr) {}

void Controller::draw(int LevelIdx, int Health, std::string_view InteractStr) {
  auto ScrSize = Scr.getSize();

  // Draw UI overlay
  Scr[0][0] << cxxg::types::Color::NONE.underline()
            << "Rogue v0.0 [FLOOR]: " << (LevelIdx + 1)
            << " [HEALTH]: " << Health;

  if (ActiveWidget) {
    ActiveWidget->draw(Scr);
    InteractStr = ActiveWidget->getInteractMsg();
  }

  if (!InteractStr.empty()) {
    Scr[2][ScrSize.X / 2 - InteractStr.size() / 2] << InteractStr;
  }
}

bool Controller::isUIActive() const { return ActiveWidget != nullptr; }

void Controller::handleInput(int Char) {
  if (!ActiveWidget) {
    return;
  }
  if (!ActiveWidget->handleInput(Char)) {
    ActiveWidget = nullptr;
  }
}

void Controller::setInventoryUI(Inventory &Inv, entt::entity Entity,
                                entt::registry &Reg) {
  ActiveWidget.reset(new InventoryController(Inv, Entity, Reg));
}

void Controller::setLootUI(Inventory &Inv, entt::entity Entity,
                           entt::registry &Reg) {
  ActiveWidget.reset(new LootController(Inv, Entity, Reg));
}

void Controller::setHistoryUI(History &Hist) {
  ActiveWidget.reset(new HistoryController(Hist));
}

} // namespace rogue::ui