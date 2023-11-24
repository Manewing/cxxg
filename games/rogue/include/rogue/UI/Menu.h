#ifndef ROGUE_UI_MENU_H
#define ROGUE_UI_MENU_H

#include <entt/entt.hpp>
#include <rogue/Components/Stats.h>
#include <rogue/UI/Decorator.h>

namespace rogue {
class Level;
} // namespace rogue

namespace rogue::ui {
class Controller;
class ItemSelect;
class TextBox;
} // namespace rogue::ui

namespace rogue::ui {

class MenuController : public BaseRectDecorator {
public:
  MenuController() = delete;
  MenuController(Controller &Ctrl, Level &Lvl);
  bool handleInput(int Char) final;
  std::string getInteractMsg() const final;
  void draw(cxxg::Screen &Scr) const final;

protected:
  Controller &Ctrl;
  Level &Lvl;
  std::shared_ptr<ItemSelect> MenuItSel;
  std::shared_ptr<TextBox> CtrlTB;
};

} // namespace rogue::ui

#endif // #define ROGUE_UI_MENU_H