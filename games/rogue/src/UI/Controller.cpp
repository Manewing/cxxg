#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <iomanip>
#include <rogue/UI/Controller.h>
#include <rogue/UI/Equipment.h>
#include <rogue/UI/History.h>
#include <rogue/UI/Inventory.h>

namespace rogue::ui {

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

cxxg::types::Size getWindowContainerSize(cxxg::Screen &Scr) {
  auto S = Scr.getSize();
  return {S.X - 1, S.Y - 2};
}

} // namespace

Controller::Controller(cxxg::Screen &Scr)
    : Scr(Scr), WdwContainer({1, 2}, getWindowContainerSize(Scr)) {}

void Controller::draw(int LevelIdx, int Health, int MaxHealth,
                      std::string_view InteractStr) {
  // Define colors
  const auto NoColor = cxxg::types::Color::NONE;
  const auto NoInterColor = cxxg::types::Color::GREY;
  const auto HasInterColor = cxxg::types::RgbColor{80, 200, 145};
  const auto HealthColor = getHealthColor(Health, MaxHealth);

  if (WdwContainer.hasActiveWindow()) {
    InteractStr = WdwContainer.getActiveWindow().getInteractMsg();
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

  WdwContainer.draw(Scr);
}

bool Controller::isUIActive() const { return WdwContainer.hasActiveWindow(); }

void Controller::handleInput(int Char) {
  if (!WdwContainer.hasActiveWindow()) {
    return;
  }
  WdwContainer.handleInput(Char);
}

void Controller::setEquipmentUI(Equipment &Equip, entt::entity Entity,
                                entt::registry &Reg) {
  WdwContainer.addWindow(std::make_shared<EquipmentController>(
      Equip, Entity, Reg, cxxg::types::Position{2, 2}));
  WdwContainer.autoLayoutWindows();
}

void Controller::setInventoryUI(Inventory &Inv, entt::entity Entity,
                                entt::registry &Reg) {
  WdwContainer.addWindow(
      std::make_shared<InventoryController>(Inv, Entity, Reg));
  WdwContainer.autoLayoutWindows();
}

void Controller::setLootUI(Inventory &Inv, entt::entity Entity,
                           entt::registry &Reg) {
  WdwContainer.addWindow(std::make_shared<LootController>(Inv, Entity, Reg));
  WdwContainer.autoLayoutWindows();
}

void Controller::setHistoryUI(History &Hist) {
  WdwContainer.addWindow(
      std::make_shared<HistoryController>(cxxg::types::Position{0, 2}, Hist));
  WdwContainer.autoLayoutWindows();
}

} // namespace rogue::ui