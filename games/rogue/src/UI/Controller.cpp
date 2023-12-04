#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <iomanip>
#include <rogue/UI/Buffs.h>
#include <rogue/UI/CommandLine.h>
#include <rogue/UI/Controller.h>
#include <rogue/UI/Equipment.h>
#include <rogue/UI/History.h>
#include <rogue/UI/Interact.h>
#include <rogue/UI/Inventory.h>
#include <rogue/UI/Menu.h>
#include <rogue/UI/Stats.h>
#include <rogue/UI/TargetUI.h>

// FIXME get rid of dep
#include <rogue/Components/Items.h>
#include <rogue/Level.h>

namespace rogue::ui {

namespace {

cxxg::types::RgbColor getHealthColor(int Health, int MaxHealth) {
  if (Health <= 0) {
    return {145, 10, 10};
  }
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

void Controller::draw(int LevelIdx, const PlayerInfo &PI,
                      const std::optional<TargetInfo> &TI) {
  // Define colors
  const auto NoColor = cxxg::types::Color::NONE;
  const auto NoInterColor = cxxg::types::Color::GREY;
  const auto HasInterColor = cxxg::types::RgbColor{80, 200, 145};
  const auto HealthColor = getHealthColor(PI.Health, PI.MaxHealth);

  auto InteractStr = PI.InteractStr;
  if (WdwContainer.hasActiveWindow()) {
    InteractStr = WdwContainer.getActiveWindow().getInteractMsg();
  }

  auto InterColor = HasInterColor;
  if (InteractStr.empty()) {
    InteractStr = "Nothing";
    InterColor = NoInterColor;
  }

  auto Acc = Scr[0][0];
  Acc << NoColor.underline() << "[FLOOR]:" << NoColor << " " << std::setw(3)
      << (LevelIdx + 1) << " | " << HealthColor << std::setw(4) << PI.Health
      << " HP" << NoColor << " | " << PI.AP << " AP | " << NoColor.underline()
      << "[T]:" << NoColor << " ";
  if (TI) {
    Acc << TI->Name << " | " << getHealthColor(TI->Health, TI->MaxHealth)
        << std::setw(4) << TI->Health << " HP" << NoColor;
  } else {
    Acc << NoInterColor.underline() << "Nothing" << NoColor;
  }
  Acc << " | " << InterColor.underline() << InteractStr;
  Acc.flushBuffer();

  WdwContainer.draw(Scr);
}

bool Controller::isUIActive() const { return WdwContainer.hasActiveWindow(); }

void Controller::handleInput(int Char) {
  if (!WdwContainer.hasActiveWindow()) {
    return;
  }
  WdwContainer.handleInput(Char);
}

void Controller::addWindow(std::shared_ptr<Widget> Wdw,
                           bool AutoLayoutWindows) {
  WdwContainer.addWindow(std::move(Wdw));
  if (AutoLayoutWindows) {
    WdwContainer.autoLayoutWindows();
  }
}

void Controller::setMenuUI(Level &Lvl) {
  WdwContainer.addWindow<MenuController>(*this, Lvl);
}

bool Controller::hasMenuUI() const {
  return WdwContainer.hasWindowOfType<MenuController>();
}

void Controller::closeMenuUI() {
  WdwContainer.closeWindow(WdwContainer.getWindowOfType<MenuController>());
}

void Controller::setCommandLineUI(Level &Lvl) {
  WdwContainer.addWindow<CommandLineController>(*this, Lvl);
  WdwContainer.getWindowOfType<CommandLineController>()->setEventHub(Hub);
}

bool Controller::hasCommandLineUI() const {
  return WdwContainer.hasWindowOfType<CommandLineController>();
}

void Controller::closeCommandLineUI() {
  WdwContainer.closeWindow(
      WdwContainer.getWindowOfType<CommandLineController>());
}

void Controller::setEquipmentUI(entt::entity Entity, entt::registry &Reg) {
  if (Entity == entt::null) {
    return;
  }
  auto &EquipComp = Reg.get<EquipmentComp>(Entity);

  WdwContainer.addWindow<EquipmentController>(*this, EquipComp.Equip, Entity,
                                              Reg, cxxg::types::Position{2, 2});
  WdwContainer.autoLayoutWindows();
}

bool Controller::hasEquipmentUI() const {
  return WdwContainer.hasWindowOfType<EquipmentController>();
}

void Controller::closeEquipmentUI() {
  WdwContainer.closeWindow(WdwContainer.getWindowOfType<EquipmentController>());
}

void Controller::setInventoryUI(entt::entity Entity, Level &Lvl) {
  if (Entity == entt::null) {
    return;
  }
  auto &InvComp = Lvl.Reg.get<InventoryComp>(Entity);
  WdwContainer.addWindow<InventoryController>(*this, InvComp.Inv, Entity, Lvl);
  WdwContainer.autoLayoutWindows();
}

bool Controller::hasInventoryUI() const {
  return WdwContainer.hasWindowOfType<InventoryController>();
}

void Controller::closeInventoryUI() {
  WdwContainer.closeWindow(WdwContainer.getWindowOfType<InventoryController>());
}

void Controller::setStatsUI(entt::entity Entity, entt::registry &Reg) {
  if (Entity == entt::null) {
    return;
  }
  auto &SC = Reg.get<StatsComp>(Entity);
  WdwContainer.addWindow<StatsController>(*this, SC, Entity, Reg);
  WdwContainer.autoLayoutWindows();
}

bool Controller::hasStatsUI() const {
  return WdwContainer.hasWindowOfType<StatsController>();
}

void Controller::closeStatsUI() {
  WdwContainer.closeWindow(WdwContainer.getWindowOfType<StatsController>());
}

void Controller::setBuffUI(entt::entity Entity, entt::registry &Reg) {
  if (Entity == entt::null) {
    return;
  }
  WdwContainer.addWindow<BuffsInfo>(Entity, Reg);
  WdwContainer.autoLayoutWindows();
}

bool Controller::hasBuffUI() const {
  return WdwContainer.hasWindowOfType<BuffsInfo>();
}

void Controller::closeBuffUI() {
  WdwContainer.closeWindow(WdwContainer.getWindowOfType<BuffsInfo>());
}

void Controller::setLootUI(entt::entity Entity, entt::entity InvEt, Level &Lvl,
                           const std::string &Header) {
  auto &InvComp = Lvl.Reg.get<InventoryComp>(InvEt);
  WdwContainer.addWindow<LootController>(*this, InvComp.Inv, Entity, Lvl,
                                         Header);
  WdwContainer.autoLayoutWindows();
}

bool Controller::hasLootUI() const {
  return WdwContainer.hasWindowOfType<LootController>();
}

void Controller::closeLootUI() {
  while (auto *Wdw = WdwContainer.getWindowOfType<LootController>()) {
    WdwContainer.closeWindow(Wdw);
  }
}

void Controller::setHistoryUI(History &Hist) {
  WdwContainer.addWindow<HistoryController>(cxxg::types::Position{0, 2}, Hist);
  WdwContainer.autoLayoutWindows();
}

bool Controller::hasHistoryUI() const {
  return WdwContainer.hasWindowOfType<HistoryController>();
}

void Controller::closeHistoryUI() {
  WdwContainer.closeWindow(WdwContainer.getWindowOfType<HistoryController>());
}

void Controller::setTargetUI(ymir::Point2d<int> TargetPos,
                             std::optional<unsigned> Range, Level &Lvl,
                             const TargetUI::SelectTargetCb &Cb) {
  closeAll();
  WdwContainer.addWindow<TargetUI>(*this, TargetPos, Range, Lvl, Cb);
  WdwContainer.autoLayoutWindows();
}

bool Controller::hasTargetUI() const {
  return WdwContainer.hasWindowOfType<TargetUI>();
}

void Controller::closeTargetUI() {
  auto *TUI = WdwContainer.getWindowOfType<TargetUI>();
  if (TUI) {
    TUI->destroyCursor();
    WdwContainer.closeWindow(TUI);
  }
}

void Controller::setInteractUI(entt::entity SrcEt, ymir::Point2d<int> StartPos,
                               Level &Lvl) {
  WdwContainer.addWindow<Interact>(SrcEt, StartPos, Lvl);
  WdwContainer.autoLayoutWindows();
}

bool Controller::hasInteractUI() const {
  return WdwContainer.hasWindowOfType<Interact>();
}

void Controller::closeInteractUI() {
  auto *Wdw = WdwContainer.getWindowOfType<Interact>();
  if (Wdw) {
    Wdw->destroyCursor();
    WdwContainer.closeWindow(Wdw);
  }
}

void Controller::closeAll() {
  while (WdwContainer.hasActiveWindow()) {
    WdwContainer.closeWindow(&WdwContainer.getActiveWindow());
  }
}

} // namespace rogue::ui