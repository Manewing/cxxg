#ifndef ROGUE_UI_INVENTORY_H
#define ROGUE_UI_INVENTORY_H

#include <entt/entt.hpp>
#include <rogue/UI/Widget.h>
#include <string_view>

namespace rogue {
class Inventory;
} // namespace rogue

namespace rogue::ui {
class ListSelect;
}

namespace rogue::ui {

class InventoryControllerBase : public Widget {
public:
  /// @brief Creates a new inventory UI controller
  /// @param Inv The inventory that is acccessed
  /// @param Entity The entity accessing the inventory
  /// @param Reg The registry the entity belongs to
  /// @param Header The header to display for the inventory
  InventoryControllerBase(Inventory &Inv, entt::entity Entity,
                          entt::registry &Reg, const std::string &Header);
  bool handleInput(int Char) override;
  void draw(cxxg::Screen &Scr) const final;

protected:
  void updateElements();

protected:
  Inventory &Inv;
  entt::entity Entity;
  entt::registry &Reg;
  std::shared_ptr<ListSelect> List;
  std::shared_ptr<Widget> Widget;
};

class InventoryController : public InventoryControllerBase {
public:
  InventoryController(Inventory &Inv, entt::entity Entity, entt::registry &Reg);
  bool handleInput(int Char) final;
  std::string_view getInteractMsg() const final;
};

class LootController : public InventoryControllerBase {
public:
  LootController(Inventory &Inv, entt::entity Entity, entt::registry &Reg);
  bool handleInput(int Char) final;
  std::string_view getInteractMsg() const final;
};

} // namespace rogue::ui

#endif // #define ROGUE_UI_INVENTORY_H