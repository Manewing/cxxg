#ifndef ROGUE_UI_CONTROLLER_H
#define ROGUE_UI_CONTROLLER_H

#include <entt/entt.hpp>
#include <memory>
#include <rogue/UI/Widget.h>
#include <string_view>

namespace cxxg {
class Screen;
}

namespace rogue {
class History;
class Inventory;
} // namespace rogue

namespace rogue::ui {

class Controller {
public:
  Controller(cxxg::Screen &Scr);

  void draw(int LevelIdx, int Health, std::string_view InteractStr);
  bool isUIActive() const;
  void handleInput(int Char);

  void setInventoryUI(Inventory &Inv, entt::entity Entity, entt::registry &Reg);
  void setLootUI(Inventory &Inv, entt::entity Entity, entt::registry &Reg);
  void setHistoryUI(History &Hist);

  std::unique_ptr<Widget> ActiveWidget;
  cxxg::Screen &Scr;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_CONTROLLER_H