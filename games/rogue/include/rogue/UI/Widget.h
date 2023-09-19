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
  explicit Widget(cxxg::types::Position Pos) : Pos(Pos) {}
  virtual ~Widget() = default;

  cxxg::types::Position getPos() const { return Pos; }
  virtual void setPos(cxxg::types::Position P) { Pos = P; }

  virtual bool handleInput(int Char) = 0;
  virtual std::string getInteractMsg() const = 0;
  virtual void draw(cxxg::Screen &Scr) const = 0;

protected:
  cxxg::types::Position Pos;
};

class BaseRect : public Widget {
public:
  BaseRect(cxxg::types::Position Pos, cxxg::types::Size Size);

  cxxg::types::Size getSize() const { return Size; }
  virtual void setSize(cxxg::types::Size S) { Size = S; }

  void draw(cxxg::Screen &Scr) const override;

protected:
  cxxg::types::Size Size;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_WIDGET_H