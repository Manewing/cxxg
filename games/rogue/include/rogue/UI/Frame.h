#ifndef ROGUE_UI_FRAME_H
#define ROGUE_UI_FRAME_H

#include <rogue/UI/Decorator.h>
#include <string>

namespace rogue::ui {

class Frame : public Decorator {
public:
  static void drawFrameHeader(cxxg::Screen &Scr, cxxg::types::Position Pos,
                              std::string_view Header, unsigned Width);
  static void drawFrameHLine(cxxg::Screen &Scr, cxxg::types::Position Pos,
                             unsigned Width);
  static void drawFrameVLine(cxxg::Screen &Scr, cxxg::types::Position Pos,
                             unsigned Width);

public:
  /// @param Size Defines the outer size of the frame
  explicit Frame(std::shared_ptr<Widget> Comp, cxxg::types::Position Pos,
                 cxxg::types::Size Size, std::string Header = "");

  void draw(cxxg::Screen &Scr) const override;

protected:
  cxxg::types::Position Pos;
  cxxg::types::Size Size;
  std::string Header;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_FRAME_H