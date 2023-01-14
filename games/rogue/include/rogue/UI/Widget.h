#ifndef ROGUE_UI_WIDGET_H
#define ROGUE_UI_WIDGET_H

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

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_WIDGET_H