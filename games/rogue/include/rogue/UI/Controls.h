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
  static constexpr KeyOption Info = {'q', "Info", "Show info about the item"};
  static constexpr KeyOption Navigate = {'^', "Navigate", "Navigate the menu"};
  static constexpr KeyOption Take = {'e', "Take", "Take the item"};
  static constexpr KeyOption Equip = {'e', "Equip", "Equip the selected item"};
  static constexpr KeyOption Unequip = {'u', "Unequip",
                                        "Unequip the selected item"};
  static constexpr KeyOption Drop = {'d', "Drop", "Drop the selected item"};
  static constexpr KeyOption Use = {
      'u', "Use", "Use the selected item, only works for consumables."};
  static constexpr KeyOption Craft = {'c', "Craft",
                                      "Use the item for crafting"};
};

} // namespace rogue::ui

#endif // #define ROGUE_UI_CONTROLS_H