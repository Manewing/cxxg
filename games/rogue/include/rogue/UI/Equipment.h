#ifndef ROGUE_UI_EQUIPMENT_H
#define ROGUE_UI_EQUIPMENT_H

#include <entt/entt.hpp>
#include <optional>
#include <rogue/UI/ItemSelect.h>
#include <rogue/UI/Widget.h>
#include <string_view>

namespace rogue {
class Equipment;
struct EquipmentSlot;
} // namespace rogue

namespace rogue::ui {

class EquipmentController : public BaseRect {
public:
  EquipmentController(Equipment &Equip, entt::entity Entity,
                      entt::registry &Reg, cxxg::types::Position Pos);
  bool handleInput(int Char) final;
  std::string_view getInteractMsg() const final;
  void draw(cxxg::Screen &Scr) const final;

protected:
  void addSelect(const EquipmentSlot &ES, cxxg::types::Position Pos);
  void updateSelectValues();

protected:
  Equipment &Equip;
  entt::entity Entity;
  entt::registry &Reg;
  std::shared_ptr<ItemSelect> ItSel;
  std::shared_ptr<Widget> Dec;
};

} // namespace rogue::ui

#endif // #define ROGUE_UI_EQUIPMENT_H