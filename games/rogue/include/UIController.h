#ifndef ROGUE_UI_CONTROLLER_H
#define ROGUE_UI_CONTROLLER_H

#include "UIListSelect.h"
#include <memory>
#include <string_view>

namespace cxxg {
class Screen;
}

class History;
class Inventory;

class UIWidget {
public:
  virtual ~UIWidget() = default;
  virtual bool handleInput(int Char) = 0;
  virtual std::string_view getInteractMsg() const = 0;
  virtual void draw(cxxg::Screen &Scr) const = 0;
};

class InventoryUIController : public UIWidget {
public:
  InventoryUIController(Inventory &Inv);
  bool handleInput(int Char) final;
  std::string_view getInteractMsg() const final;
  void draw(cxxg::Screen &Scr) const final;

protected:
  void updateElements();

protected:
  Inventory &Inv;
  UIListSelect ListUI;
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

  // FIXME use dependency injection
  void setInventoryUI(Inventory &Inv);
  void setHistoryUI(History &Hist);

  std::unique_ptr<UIWidget> ActiveWidget;
  cxxg::Screen &Scr;
};

#endif // #ifndef ROGUE_UI_CONTROLLER_H