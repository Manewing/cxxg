#ifndef ROGUE_UI_CONTROLS_H
#define ROGUE_UI_CONTROLS_H

#include <string>
#include <vector>

namespace rogue::ui {

struct KeyOption {
  int Char = -1;
  std::string_view Help = "<help-missing>";
  std::string_view Desc = "<description-missing>";

  static std::string getInteractMsg(const std::vector<KeyOption> &Options);
};

struct Controls {
  // UI controls
  static constexpr KeyOption InventoryUI = {'i', "Inventory", "Open inventory"};
  static constexpr KeyOption CharacterUI = {'c', "Character", "Open character"};
  static constexpr KeyOption EquipmentUI = {'o', "Equipment", "Open equipment"};
  static constexpr KeyOption HistoryUI = {'h', "History", "Open history"};
  static constexpr KeyOption BuffsUI = {'b', "Buffs", "Open buffs"};
  static constexpr KeyOption TargetUI = {'t', "Target",
                                         "Target object for info/attack"};

  // Game controls
  static constexpr KeyOption MoveUp = {'w', "Move up", "Move up"};
  static constexpr KeyOption MoveDown = {'s', "Move down", "Move down"};
  static constexpr KeyOption MoveLeft = {'a', "Move left", "Move left"};
  static constexpr KeyOption MoveRight = {'d', "Move right", "Move right"};

  // Inventory controls
  static constexpr KeyOption Info = {'q', "Info", "Show info about the item"};
  static constexpr KeyOption Navigate = {'^', "Navigate", "Navigate the menu"};
  static constexpr KeyOption Take = {'e', "Take", "Take the item"};
  static constexpr KeyOption SpendPoint = {'e', "Spend", "Spend a point"};
  static constexpr KeyOption Equip = {'e', "Equip", "Equip the selected item"};
  static constexpr KeyOption Unequip = {'u', "Unequip",
                                        "Unequip the selected item"};
  static constexpr KeyOption Drop = {'d', "Drop", "Drop the selected item"};
  static constexpr KeyOption Use = {
      'u', "Use", "Use the selected item, only works for consumables."};
  static constexpr KeyOption Craft = {'x', "Craft",
                                      "Use the item for crafting"};
};

} // namespace rogue::ui

#endif // #define ROGUE_UI_CONTROLS_H