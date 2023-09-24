#ifndef ROGUE_UI_FRAME_H
#define ROGUE_UI_FRAME_H

#include <cxxg/Types.h>
#include <rogue/UI/Decorator.h>
#include <string>

namespace rogue::ui {

class Frame : public Decorator {
public:
  static void drawFrameHeader(cxxg::Screen &Scr, cxxg::types::Position Pos,
                              std::string_view Header, unsigned Width,
                              cxxg::types::TermColor FrameColor,
                              cxxg::types::TermColor HeaderColor);
  static void drawFrameHLine(cxxg::Screen &Scr, cxxg::types::Position Pos,
                             unsigned Width, cxxg::types::TermColor Color);
  static void drawFrameVLine(cxxg::Screen &Scr, cxxg::types::Position Pos,
                             unsigned Width, cxxg::types::TermColor Color);

public:
  /// @param Size Defines the outer size of the frame
  explicit Frame(std::shared_ptr<Widget> Comp, cxxg::types::Position Pos,
                 cxxg::types::Size Size, std::string Header = "");

  void setFrameColor(cxxg::types::TermColor Color);
  cxxg::types::TermColor getFrameColor() const { return FrameColor; }
  void setHeaderColor(cxxg::types::TermColor Color);
  cxxg::types::TermColor getHeaderColor() const { return HeaderColor; }

  void draw(cxxg::Screen &Scr) const override;

protected:
  cxxg::types::Size Size;
  std::string Header;
  cxxg::types::TermColor FrameColor = cxxg::types::Color::NONE;
  cxxg::types::TermColor HeaderColor = cxxg::types::Color::NONE;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_FRAME_H