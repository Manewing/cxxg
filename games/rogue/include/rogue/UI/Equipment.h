#ifndef ROGUE_UI_EQUIPMENT_H
#define ROGUE_UI_EQUIPMENT_H

#include <entt/entt.hpp>
#include <rogue/UI/ListSelect.h>
#include <rogue/UI/Widget.h>
#include <string_view>

namespace rogue {
class Inventory;
} // namespace rogue

namespace rogue::ui {

class EquipmentController : public Widget {
public:
  EquipmentController(entt::entity Entity, entt::registry &Reg);
  bool handleInput(int Char) override;
  void draw(cxxg::Screen &Scr) const final;

protected:
  void updateElements();

protected:
  Inventory &Inv;
  entt::entity Entity;
  entt::registry &Reg;
  ListSelect List;
};

} // namespace rogue::ui

#endif // #define ROGUE_UI_EQUIPMENT_H