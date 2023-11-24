#include <array>
#include <cxxg/Screen.h>
#include <rogue/UI/Controller.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/ItemSelect.h>
#include <rogue/UI/Menu.h>
#include <rogue/UI/TextBox.h>

static constexpr cxxg::types::Position DefaultPos = {2, 2};
static constexpr cxxg::types::Size DefaultSize = {60, 20};

struct MenuItemInfo {
  cxxg::types::Position Offset = {0, 0};
  const char *Text = "<unimp. text>";
};
static constexpr std::array<MenuItemInfo, 2> MenuItemInfos = {{
    {{(DefaultSize.X - 6) / 2, 2}, "Resume"},
    {{(DefaultSize.X - 8) / 2, 4}, "Controls"},
}};

static constexpr std::array<const rogue::ui::KeyOption *, 6> GameCtrlInfos = {
    &rogue::ui::Controls::MoveUp,
    &rogue::ui::Controls::MoveDown,
    &rogue::ui::Controls::MoveLeft,
    &rogue::ui::Controls::MoveRight,
    &rogue::ui::Controls::Interact,
    &rogue::ui::Controls::Rest,
};


static constexpr std::array<const rogue::ui::KeyOption *, 10> UICtrlInfos = {
    &rogue::ui::Controls::InventoryUI,
    &rogue::ui::Controls::CharacterUI,
    &rogue::ui::Controls::EquipmentUI,
    &rogue::ui::Controls::HistoryUI,
    &rogue::ui::Controls::BuffsUI,
    &rogue::ui::Controls::TargetUI,
    &rogue::ui::Controls::MoveWindow,
    &rogue::ui::Controls::NextWindow,
    &rogue::ui::Controls::PrevWindow,
    &rogue::ui::Controls::CloseWindow,
};

namespace rogue::ui {

MenuController::MenuController(Controller &C)
    : BaseRectDecorator(DefaultPos, DefaultSize, nullptr), Ctrl(C) {
  MenuItSel = std::make_shared<ItemSelect>(Pos);

  for (const auto &SI : MenuItemInfos) {
    MenuItSel->addSelect<Select>(SI.Text, SI.Offset, 8);
  }

  CtrlTB = std::make_shared<TextBox>(Pos, DefaultSize, "Controls");
  std::stringstream SS;
  SS << "--- Game ---\n";
  for (const auto *C : GameCtrlInfos) {
    SS << C->getInteractMsg() << "\n" << C->Desc << "\n\n";
  }
  SS << "--- UI ---\n";
  for (const auto *C : UICtrlInfos) {
    SS << C->getInteractMsg() << "\n" << C->Desc << "\n\n";
  }
  CtrlTB->setText(SS.str());

  Comp = std::make_shared<Frame>(MenuItSel, Pos, DefaultSize, "Menu");

  MenuItSel->registerOnSelectCallback([this](const Select &S) {
    if (S.getValue() == "Resume") {
      Ctrl.closeMenuUI();
    } else if (S.getValue() == "Controls") {
      static_cast<Frame *>(Comp.get())->setComp(CtrlTB);
    }
  });
}

bool MenuController::handleInput(int Char) {
  switch (Char) {
  default:
    return BaseRectDecorator::handleInput(Char);
  }
  return true;
}

std::string MenuController::getInteractMsg() const {
  std::vector<KeyOption> Options = {Controls::Navigate};
  return KeyOption::getInteractMsg(Options);
}

void MenuController::draw(cxxg::Screen &Scr) const {
  BaseRectDecorator::draw(Scr);
}

} // namespace rogue::ui