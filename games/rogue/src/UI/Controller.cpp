#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <iomanip>
#include <rogue/UI/Controller.h>
#include <rogue/UI/Equipment.h>
#include <rogue/UI/History.h>
#include <rogue/UI/Inventory.h>

namespace rogue::ui {

Controller::Controller(cxxg::Screen &Scr) : Scr(Scr) {}

namespace {

cxxg::types::RgbColor getHealthColor(int Health, int MaxHealth) {
  int Percent = (Health * 100) / MaxHealth;
  if (Percent >= 66) {
    return {135, 250, 10};
  }
  if (Percent >= 33) {
    return {225, 140, 10};
  }
  return {245, 30, 30};
}

} // namespace

void Controller::draw(int LevelIdx, int Health, int MaxHealth,
                      std::string_view InteractStr) {
  // Define colors
  const auto NoColor = cxxg::types::Color::NONE;
  const auto NoInterColor = cxxg::types::Color::GREY;
  const auto HasInterColor = cxxg::types::RgbColor{80, 200, 145};
  const auto HealthColor = getHealthColor(Health, MaxHealth);

  if (ActiveWidget) {
    InteractStr = ActiveWidget->getInteractMsg();
  }

  auto InterColor = HasInterColor;
  auto InterStr = InteractStr;
  if (InterStr.empty()) {
    InterStr = "Nothing";
    InterColor = NoInterColor;
  }

  Scr[0][0] << NoColor.underline() << "Rogue v0.0 [FLOOR]:" << NoColor << " "
            << std::setw(3) << (LevelIdx + 1) << " " << NoColor.underline()
            << "[HEALTH]:" << HealthColor << std::setw(4) << Health << NoColor
            << " | " << InterColor.underline() << InterStr;

  if (ActiveWidget) {
    ActiveWidget->draw(Scr);
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

void Controller::setEquipmentUI(Equipment &Equip, entt::entity Entity,
                                entt::registry &Reg) {
  ActiveWidget.reset(new EquipmentController(Equip, Entity, Reg, {2, 2}));
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