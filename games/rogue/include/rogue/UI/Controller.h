#ifndef ROGUE_UI_CONTROLLER_H
#define ROGUE_UI_CONTROLLER_H

#include <entt/entt.hpp>
#include <memory>
#include <rogue/UI/Widget.h>
#include <rogue/UI/WindowContainer.h>
#include <string_view>

namespace cxxg {
class Screen;
}

namespace rogue {
class History;
class Inventory;
class Equipment;
} // namespace rogue

namespace rogue::ui {

class Controller {
public:
  Controller(cxxg::Screen &Scr);
  void draw(int LevelIdx, int Health, int MaxHealth,
            std::string_view InteractStr);
  bool isUIActive() const;
  void handleInput(int Char);

  void setEquipmentUI(Equipment &Equip, entt::entity Entity,
                      entt::registry &Reg);
  bool hasEquipmentUI() const;
  void closeEquipmentUI();

  void setInventoryUI(Inventory &Inv, entt::entity Entity, entt::registry &Reg);
  bool hasInventoryUI() const;
  void closeInventoryUI();

  void setLootUI(Inventory &Inv, entt::entity Entity, entt::registry &Reg);
  void setHistoryUI(History &Hist);

private:
  cxxg::Screen &Scr;
  WindowContainer WdwContainer;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_CONTROLLER_H