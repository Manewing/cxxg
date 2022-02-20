#ifndef ROGUE_UI_CONTROLLER_H
#define ROGUE_UI_CONTROLLER_H

#include <string_view>
#include "UIListSelect.h"
#include <memory>

namespace cxxg {
class Screen;
}

class Inventory;

class InventoryUIController {
public:
  InventoryUIController(Inventory &Inv);
  bool handleInput(int Char);
  std::string_view getInteractMsg() const;
  void draw(cxxg::Screen &Scr) const;

protected:
  void updateElements();

protected:
  Inventory &Inv;
  UIListSelect ListUI;
};

class UIController {
public:
  UIController(cxxg::Screen &Scr);

  void draw(int LevelIdx, int Health, std::string_view InteractStr);
  bool isUIActive() const;
  void handleInput(int Char);
  void setInventoryUI(Inventory &Inv);


  std::unique_ptr<InventoryUIController> InventoryUI;
  cxxg::Screen &Scr;
};

#endif // #ifndef ROGUE_UI_CONTROLLER_H