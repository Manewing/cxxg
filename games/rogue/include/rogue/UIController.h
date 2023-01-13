#ifndef ROGUE_UI_CONTROLLER_H
#define ROGUE_UI_CONTROLLER_H

#include "UIListSelect.h"
#include <entt/entt.hpp>
#include <memory>
#include <string_view>

namespace cxxg {
class Screen;
}

namespace rogue {
class History;
class Inventory;
} // namespace rogue

namespace rogue {

class UIWidget {
public:
  virtual ~UIWidget() = default;
  virtual bool handleInput(int Char) = 0;
  virtual std::string_view getInteractMsg() const = 0;
  virtual void draw(cxxg::Screen &Scr) const = 0;
};

class InventoryUIControllerBase : public UIWidget {
public:
  /// @brief Creates a new inventory UI controller
  /// @param Inv The inventory that is acccessed
  /// @param Entity The entity accessing the inventory
  /// @param Reg The registry the entity belongs to
  /// @param Header The header to display for the inventory
  InventoryUIControllerBase(Inventory &Inv, entt::entity Entity,
                            entt::registry &Reg, const std::string &Header);
  bool handleInput(int Char) override;
  void draw(cxxg::Screen &Scr) const final;

protected:
  void updateElements();

protected:
  Inventory &Inv;
  entt::entity Entity;
  entt::registry &Reg;
  UIListSelect ListUI;
};

class InventoryUIController : public InventoryUIControllerBase {
public:
  InventoryUIController(Inventory &Inv, entt::entity Entity,
                        entt::registry &Reg);
  bool handleInput(int Char) final;
  std::string_view getInteractMsg() const final;
};

class LootUIController : public InventoryUIControllerBase {
public:
  LootUIController(Inventory &Inv, entt::entity Entity, entt::registry &Reg);
  bool handleInput(int Char) final;
  std::string_view getInteractMsg() const final;
};

class HistoryUIController : public UIWidget {
public:
  HistoryUIController(const History &Hist, unsigned NumHistoryRows = 18);
  bool handleInput(int Char) final;
  std::string_view getInteractMsg() const final;
  void draw(cxxg::Screen &Scr) const final;

protected:
  const History &Hist;
  const unsigned NumHistoryRows;
  unsigned Offset = 0;
};

class UIController {
public:
  UIController(cxxg::Screen &Scr);

  void draw(int LevelIdx, int Health, std::string_view InteractStr);
  bool isUIActive() const;
  void handleInput(int Char);

  void setInventoryUI(Inventory &Inv, entt::entity Entity, entt::registry &Reg);
  void setLootUI(Inventory &Inv, entt::entity Entity, entt::registry &Reg);
  void setHistoryUI(History &Hist);

  std::unique_ptr<UIWidget> ActiveWidget;
  cxxg::Screen &Scr;
};

} // namespace rogue

#endif // #ifndef ROGUE_UI_CONTROLLER_H