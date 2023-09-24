#ifndef ROGUE_UI_EQUIPMENT_H
#define ROGUE_UI_EQUIPMENT_H

#include <entt/entt.hpp>
#include <optional>
#include <rogue/UI/Decorator.h>
#include <rogue/UI/ItemSelect.h>
#include <rogue/UI/Widget.h>
#include <string_view>

namespace rogue {
class Equipment;
class Controller;
struct EquipmentSlot;
} // namespace rogue

namespace rogue::ui {

class EquipmentController : public BaseRectDecorator {
public:
  EquipmentController(Controller &Ctrl, Equipment &Equip, entt::entity Entity,
                      entt::registry &Reg, cxxg::types::Position Pos);
  bool handleInput(int Char) final;
  std::string getInteractMsg() const final;
  void draw(cxxg::Screen &Scr) const final;

protected:
  void addSelect(const EquipmentSlot &ES, cxxg::types::Position Pos);
  void updateSelectValues() const;

protected:
  Controller &Ctrl;
  Equipment &Equip;
  entt::entity Entity;
  entt::registry &Reg;
  std::shared_ptr<ItemSelect> ItSel;
};

} // namespace rogue::ui

#endif // #define ROGUE_UI_EQUIPMENT_H