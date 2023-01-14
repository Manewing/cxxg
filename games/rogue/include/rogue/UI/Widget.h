#ifndef ROGUE_UI_WIDGET_H
#define ROGUE_UI_WIDGET_H

#include <cxxg/Types.h>
#include <string_view>

namespace cxxg {
class Screen;
}

namespace rogue::ui {

class Widget {
public:
  virtual ~Widget() = default;
  virtual bool handleInput(int Char) = 0;
  virtual std::string_view getInteractMsg() const = 0;
  virtual void draw(cxxg::Screen &Scr) const = 0;
};

class BaseRect : public Widget {
public:
  BaseRect(cxxg::types::Position Pos, cxxg::types::Size Size);
  void draw(cxxg::Screen &Scr) const override;

protected:
  cxxg::types::Position Pos;
  cxxg::types::Size Size;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_WIDGET_H