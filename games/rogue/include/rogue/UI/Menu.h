#ifndef ROGUE_UI_MENU_H
#define ROGUE_UI_MENU_H

#include <entt/entt.hpp>
#include <rogue/Components/Stats.h>
#include <rogue/UI/Controller.h>
#include <rogue/UI/Decorator.h>

namespace rogue {
class Level;
} // namespace rogue

namespace rogue::ui {
class ItemSelect;
class TextBox;
} // namespace rogue::ui

namespace rogue::ui {

class MenuController : public BaseRectDecorator {
public:
  using LoadGameCbTy = Controller::LoadGameCbTy;
  using SaveGameCbTy = Controller::SaveGameCbTy;

public:
  MenuController() = delete;
  MenuController(Controller &Ctrl, Level &Lvl, LoadGameCbTy LoadGameCb, SaveGameCbTy SaveGameCb); 
  bool handleInput(int Char) final;
  std::string getInteractMsg() const final;
  void draw(cxxg::Screen &Scr) const final;

protected:
  Controller &Ctrl;
  Level &Lvl;
  LoadGameCbTy LoadGameCb;
  SaveGameCbTy SaveGameCb;
  std::shared_ptr<ItemSelect> MenuItSel;
  std::shared_ptr<TextBox> CtrlTB;
};

} // namespace rogue::ui

#endif // #define ROGUE_UI_MENU_H