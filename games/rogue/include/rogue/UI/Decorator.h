#ifndef ROGUE_UI_DECORATOR_H
#define ROGUE_UI_DECORATOR_H

#include <cxxg/Types.h>
#include <memory>
#include <rogue/UI/Widget.h>

namespace rogue::ui {

class Decorator : public Widget {
public:
  Decorator(cxxg::types::Position Pos, std::shared_ptr<Widget> Comp);

  void setPos(cxxg::types::Position Pos) override;
  bool handleInput(int Char) override;
  std::string_view getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

private:
  std::shared_ptr<Widget> Comp;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_DECORATOR_H