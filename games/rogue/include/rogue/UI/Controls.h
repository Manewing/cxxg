#ifndef ROGUE_UI_CONTROLS_H
#define ROGUE_UI_CONTROLS_H

#include <cxxg/Utils.h>
#include <string>
#include <vector>

namespace rogue::ui {

struct KeyOption {
  int Char = -1;
  std::string_view Help = "<help-missing>";
  std::string_view Desc = "<description-missing>";

  static const char *getCharStr(int Char);
  static std::string getInteractMsg(const std::vector<KeyOption> &Options);
  inline std::string getInteractMsg() const { return getInteractMsg({*this}); }
};

struct Controls {
  // UI controls
  static constexpr KeyOption InventoryUI = {'i', "Inventory", "Open inventory"};
  static constexpr KeyOption CharacterUI = {'c', "Character", "Open character"};
  static constexpr KeyOption EquipmentUI = {'o', "Equipment", "Open equipment"};
  static constexpr KeyOption HistoryUI = {'h', "History", "Open history"};
  static constexpr KeyOption BuffsUI = {'b', "Buffs", "Open buffs"};
  static constexpr KeyOption TargetUI = {'t', "Target",
                                         "Target object for info or attack"};

  // Window controls
  static constexpr KeyOption MoveWindow = {'m', "Move", "Move the window"};
  static constexpr KeyOption NextWindow = {cxxg::utils::KEY_TAB, "Next",
                                           "Select next window"};
  static constexpr KeyOption PrevWindow = {'V', "Prev", "Select prev window"};
  static constexpr KeyOption AutoLayout = {'X', "Auto", "Auto layout windows"};
  static constexpr KeyOption CloseWindow = {cxxg::utils::KEY_ESC, "Close",
                                            "Close the window"};

  // Game controls
  static constexpr KeyOption MoveUp = {cxxg::utils::KEY_UP, "Move", "Move up"};
  static constexpr KeyOption MoveDown = {cxxg::utils::KEY_DOWN, "",
                                         "Move down"};
  static constexpr KeyOption MoveLeft = {cxxg::utils::KEY_LEFT, "",
                                         "Move left"};
  static constexpr KeyOption MoveRight = {cxxg::utils::KEY_RIGHT, "",
                                          "Move right"};
  static constexpr KeyOption Interact = {'e', "Interact", "Interact"};
  static constexpr KeyOption Rest = {' ', "Rest", "Rest"};

  // Inventory controls
  static constexpr KeyOption Info = {'q', "Info", "Show info about the item"};
  static constexpr KeyOption Navigate = {'^', "Nav.", "Navigate up/down"};
  static constexpr KeyOption Take = {'e', "Take", "Take the item"};
  static constexpr KeyOption SpendPoint = {'e', "Spend", "Spend a point"};
  static constexpr KeyOption Equip = {'e', "Equip", "Equip the selected item"};
  static constexpr KeyOption Unequip = {'u', "Unequip",
                                        "Unequip the selected item"};
  static constexpr KeyOption Drop = {'d', "Drop", "Drop the selected item"};
  static constexpr KeyOption Dismantle = {'a', "Dismantle",
                                          "Dismantle the selected item"};
  static constexpr KeyOption Store = {'s', "Store", "Store the selected item"};
  static constexpr KeyOption Use = {
      'u', "Use", "Use the selected item, only works for consumables."};
  static constexpr KeyOption SelectTarget = {
      ' ', "Sel.", "Select the current target"};
  static constexpr KeyOption Craft = {'x', "Craft",
                                      "Use the item for crafting"};

  static int getRemappedChar(int Char);
};

} // namespace rogue::ui

#endif // #define ROGUE_UI_CONTROLS_H