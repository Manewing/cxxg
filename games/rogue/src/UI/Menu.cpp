#include <array>
#include <cxxg/Screen.h>
#include <rogue/Level.h>
#include <rogue/Serialization.h>
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
  bool Debug = false;
};
static constexpr std::array<MenuItemInfo, 8> MenuItemInfos = {{
    {{(DefaultSize.X - 6) / 2, 2}, "Resume", false},
    {{(DefaultSize.X - 8) / 2, 4}, "Controls", false},
    {{(DefaultSize.X - 4) / 2, 6}, "Help", false},
    {{(DefaultSize.X - 11) / 2, 8}, "Help Combat", false},
    {{(DefaultSize.X - 13) / 2, 10}, "Help Crafting", false},
    {{(DefaultSize.X - 9) / 2, 12}, "Save Game", false},
    {{(DefaultSize.X - 9) / 2, 14}, "Load Game", false},
    {{(DefaultSize.X - 12) / 2, 16}, "Command Line", true},
}};

static constexpr std::array<const rogue::ui::KeyOption *, 6> GameCtrlInfos = {
    &rogue::ui::Controls::MoveUp,   &rogue::ui::Controls::MoveDown,
    &rogue::ui::Controls::MoveLeft, &rogue::ui::Controls::MoveRight,
    &rogue::ui::Controls::Interact, &rogue::ui::Controls::Rest,
};

static constexpr std::array<const rogue::ui::KeyOption *, 10> UICtrlInfos = {
    &rogue::ui::Controls::InventoryUI, &rogue::ui::Controls::CharacterUI,
    &rogue::ui::Controls::EquipmentUI, &rogue::ui::Controls::HistoryUI,
    &rogue::ui::Controls::BuffsUI,     &rogue::ui::Controls::TargetUI,
    &rogue::ui::Controls::MoveWindow,  &rogue::ui::Controls::NextWindow,
    &rogue::ui::Controls::PrevWindow,  &rogue::ui::Controls::CloseWindow,
};

namespace rogue::ui {

namespace {

std::shared_ptr<Widget> makeHelpWindow(const std::string &Title,
                                       const std::string &Text,
                                       const cxxg::types::Position &Pos,
                                       const cxxg::types::Size &Size) {
  std::shared_ptr<Widget> HelpTB = std::make_shared<TextBox>(Pos, Size, Text);
  HelpTB = std::make_shared<Frame>(HelpTB, Pos, Size, Title);
  return HelpTB;
}

std::shared_ptr<Widget> makeHelpWindow() {
  std::stringstream SS;
  SS << "Welcome to Rogue\n"
     << "\n"
     << "Rogue is a turn based rogue-like dungeon crawler. Your goal is to "
        "find the Final Key and escape the dungeon.\n"
     << "\n"
     << "You will find the key for the first level in your Stash. Keys for the "
        "subsequent levels can be found in dungeons.\n"
     << "\n"
     << "You can move around using the arrow keys and interact with "
        "entities using ("
     << Controls::Interact.getInteractMsg() << ").\n"
     << "\n"
     << "At the top of the screen you can see:\n"
     << " - The current level\n"
     << " - Your health (HP)\n"
     << " - Your mana (MP)\n"
     << " - Information on your current target ([T])\n"
     << " - Possible actions and their keybindings\n"
     << "\n"
     << "Progress is heavily based on your equipment. So make sure to upgrade "
        "your equipment regularly from found items, by crafting or by "
        "enhancing your items. You can equip items using ("
     << Controls::Equip.getInteractMsg() << ").\n\n"
     << "Equipment is also the only way to get access to new skills. You can "
        "use skills by pressing the corresponding number key (0-9). You can "
        "see the skills of your currently equipped items in the equipment "
        "window. Items without skill will show '[-]'.\n";

  return makeHelpWindow("Help", SS.str(), DefaultPos, DefaultSize);
}

std::shared_ptr<Widget> makeHelpCombatWindow() {
  std::stringstream SS;
  SS << "Combat is turn based. You can move around and attack enemies. "
        "Enemies will also move around and attack you.\n"
     << "\n"
     << "You can attack enemies by moving into them. You can also attack "
        "enemies from a distance by using ranged weapons or spells. For this "
        "use ("
     << Controls::TargetUI.getInteractMsg() << ")\n"
     << "\n"
     << "You can use items from the inventory using ("
     << Controls::Use.getInteractMsg()
     << "). Most items will be used on yourself, though some items will allow "
        "you to target another entity.\n"
     << "\n"
     << "You can rest using (" << Controls::Rest.getInteractMsg()
     << "). This will restore your health "
        "and mana.\n"
     << "\n";
  return makeHelpWindow("Help Combat", SS.str(), DefaultPos, DefaultSize);
}

std::shared_ptr<Widget> makeHelpCraftingWindow() {
  std::stringstream SS;
  SS << "Crafting can be done at the workbench and allows you to create new "
        "items. There are two types of crafting:\n"
     << "\n"
     << "Recipes:\n"
     << "These are predefined recipes that you can use to craft "
        "items. You can see the known recipes at the workbench, you can also "
        "craft any known recipe there directly if you have the required "
        "items.\n"
     << "Recipes can be learned by trying to craft item using the "
        "workbench or by special items that teach crafting recipes. You can "
        "always craft a recipe from the workbench even if you have not learned "
        "it, by adding the correct ingredients.\n"
     << "\n"
     << "Enhancements:\n"
     << "Certain items (Crafting Base, Equipment) can be enhanced by adding "
        "other items to them. This will add the properties of the enhancement "
        "item to the base item. The final properties of the enhanced item "
        "depend on the order of the added items and their respective effects "
        "and capabilities. (For example consumable properties can not be added "
        "to equipment).";

  return makeHelpWindow("Help Crafting", SS.str(), DefaultPos, DefaultSize);
}

std::shared_ptr<Widget> makeSlotsWindow(
    const std::string &Title, bool StoreAsJSON,
    const std::function<void(const SaveGameInfo &SGI)> &SlotSelectCb) {
  cxxg::types::Position Pos = {2, 2};
  static constexpr auto SlotWdwPos = cxxg::types::Position{2, 2};
  static constexpr auto SlotWdwSize = cxxg::types::Size{30, 12};

  std::shared_ptr<ItemSelect> SlotSelect = std::make_shared<ItemSelect>(Pos);

  std::string SlotStr = "Slot 0";

  std::map<std::string, SaveGameInfo> SgMap;
  for (unsigned SlotIdx = 0; SlotIdx < 5; SlotIdx++) {
    SlotStr[5] = '1' + SlotIdx;

    auto SGI = SaveGameInfo::fromSlot(SlotIdx, StoreAsJSON);
    SgMap[SlotStr] = SGI;

    std::string Label = "Empty";
    cxxg::types::TermColor LabelColor = cxxg::types::Color::GREY;
    if (SGI.exists()) {
      Label = SGI.getDateStr();
      LabelColor = cxxg::types::Color::GREEN;
    }

    auto SlotPos = Pos;
    SlotPos.Y += SlotIdx * 2;
    SlotSelect->addSelect<LabeledSelect>(
        SlotStr, Label, SlotPos, SlotWdwSize.X - 2, LabelColor, LabelColor);
  }

  SlotSelect->registerOnSelectCallback([SgMap, SlotSelectCb](const Select &S) {
    auto &LS = static_cast<const LabeledSelect &>(S);
    SlotSelectCb(SgMap.at(LS.getLabel()));
  });

  std::shared_ptr<Widget> Wdw =
      std::make_shared<BaseRectDecorator>(SlotWdwPos, SlotWdwSize, SlotSelect);
  Wdw = std::make_shared<Frame>(Wdw, SlotWdwPos, SlotWdwSize, Title);

  return Wdw;
}

std::shared_ptr<Widget>
makeLoadSlotWindow(Controller &Ctrl, MenuController::LoadGameCbTy LoadGameCb,
                   bool StoreAsJSON) {
  return makeSlotsWindow(
      "Load Game", StoreAsJSON, [LoadGameCb, &Ctrl](const auto SGI) mutable {
        if (!SGI.exists()) {
          return;
        }
        try {
          LoadGameCb(SGI);
          Ctrl.tooltip("Loaded game", "Info");
        } catch (const std::exception &E) {
          Ctrl.tooltip("Failed to load game from " + SGI.Path.string() + ":\n" +
                           E.what(),
                       "Error");
        }
      });
}

std::shared_ptr<Widget>
makeSaveSlotWindow(Controller &Ctrl, MenuController::SaveGameCbTy SaveGameCb,
                   bool StoreAsJSON) {
  return makeSlotsWindow(
      "Save Game", StoreAsJSON, [SaveGameCb, &Ctrl](const auto SGI) mutable {
        try {
          SaveGameCb(SGI);
          Ctrl.tooltip("Saved game", "Info");
        } catch (const std::exception &E) {
          Ctrl.tooltip("Failed to save game to" + SGI.Path.string() + ":\n" +
                           E.what(),
                       "Error");
        }
      });
}

} // namespace

MenuController::MenuController(Controller &C, Level &L, LoadGameCbTy Ld,
                               SaveGameCbTy Sv)
    : BaseRectDecorator(DefaultPos, DefaultSize, nullptr), Ctrl(C), Lvl(L),
      LoadGameCb(std::move(Ld)), SaveGameCb(std::move(Sv)) {
  MenuItSel = std::make_shared<ItemSelect>(Pos);

  const bool Debug = getenv("ROGUE_DEBUG") != nullptr;
  for (const auto &SI : MenuItemInfos) {
    if (SI.Debug && !Debug) {
      continue;
    }
    MenuItSel->addSelect<Select>(SI.Text, SI.Offset, 14);
  }

  CtrlTB = std::make_shared<TextBox>(Pos, DefaultSize, "Controls");
  std::stringstream SS;
  SS << "--- Game ---\n";
  for (const auto *C : GameCtrlInfos) {
    SS << C->getInteractMsg() << "\n" << C->Desc << "\n\n";
  }
  SS << "[0-9] Skill\nCast skill of related item in equipment slot.\n\n";
  SS << "--- UI ---\n";
  for (const auto *C : UICtrlInfos) {
    SS << C->getInteractMsg() << "\n" << C->Desc << "\n\n";
  }
  CtrlTB->setText(SS.str());

  Comp = std::make_shared<Frame>(MenuItSel, Pos, DefaultSize, "Menu");

  static const bool StoreAsJSON = true;

  MenuItSel->registerOnSelectCallback([this](const Select &S) {
    if (S.getValue() == "Resume") {
      Ctrl.closeMenuUI();
    } else if (S.getValue() == "Controls") {
      CtrlTB->setPos(Pos);
      static_cast<Frame *>(Comp.get())->setComp(CtrlTB);
    } else if (S.getValue() == "Help") {
      Ctrl.addWindow(makeHelpWindow(), false, true);
      Ctrl.closeMenuUI();
    } else if (S.getValue() == "Help Combat") {
      Ctrl.addWindow(makeHelpCombatWindow(), false, true);
      Ctrl.closeMenuUI();
    } else if (S.getValue() == "Help Crafting") {
      Ctrl.addWindow(makeHelpCraftingWindow(), false, true);
      Ctrl.closeMenuUI();
    } else if (S.getValue() == "Load Game") {
      Ctrl.addWindow(makeLoadSlotWindow(Ctrl, LoadGameCb, StoreAsJSON), false,
                     true);
      Ctrl.closeMenuUI();
    } else if (S.getValue() == "Save Game") {
      Ctrl.addWindow(makeSaveSlotWindow(Ctrl, SaveGameCb, StoreAsJSON), false,
                     true);
      Ctrl.closeMenuUI();
    } else if (S.getValue() == "Command Line") {
      Ctrl.setCommandLineUI(Lvl);
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