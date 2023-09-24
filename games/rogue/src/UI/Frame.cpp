#include <cxxg/Screen.h>
#include <rogue/UI/Frame.h>

static constexpr auto NoColor = cxxg::types::Color::NONE;

namespace rogue::ui {

void Frame::drawFrameHeader(cxxg::Screen &Scr, cxxg::types::Position Pos,
                            std::string_view Header, unsigned Width,
                            cxxg::types::TermColor FrameColor,
                            cxxg::types::TermColor HeaderColor) {
  drawFrameHLine(Scr, Pos, Width, FrameColor);
  unsigned HdrOffset = (Width - Header.size()) / 2 - 1;
  Scr[Pos.Y][Pos.X + HdrOffset] << FrameColor << "[" << HeaderColor << Header
                                << FrameColor << "]";
}

void Frame::drawFrameHLine(cxxg::Screen &Scr, cxxg::types::Position Pos,
                           unsigned Width, cxxg::types::TermColor Color) {
  Scr[Pos.Y][Pos.X] << Color << "+" << std::string(Width - 2, '-') << "+"
                    << NoColor;
}

void Frame::drawFrameVLine(cxxg::Screen &Scr, cxxg::types::Position Pos,
                           unsigned Width, cxxg::types::TermColor Color) {
  Scr[Pos.Y][Pos.X] = '|';
  Scr[Pos.Y][Pos.X] = Color;
  Scr[Pos.Y][Pos.X + Width - 1] = '|';
  Scr[Pos.Y][Pos.X + Width - 1] = Color;
}

Frame::Frame(std::shared_ptr<Widget> Comp, cxxg::types::Position Pos,
             cxxg::types::Size Size, std::string Header)
    : Decorator(Pos, std::move(Comp)), Size(Size), Header(std::move(Header)) {}

void Frame::setFrameColor(cxxg::types::TermColor Color) { FrameColor = Color; }

void Frame::setHeaderColor(cxxg::types::TermColor Color) {
  HeaderColor = Color;
}

void Frame::draw(cxxg::Screen &Scr) const {
  Decorator::draw(Scr);
  // Draw frame
  if (Header.empty()) {
    drawFrameHLine(Scr, {Pos.X, Pos.Y}, Size.X, FrameColor);
  } else {
    drawFrameHeader(Scr, {Pos.X, Pos.Y}, Header, Size.X, FrameColor,
                    HeaderColor);
  }
  for (unsigned Row = 0; Row < Size.Y - 2; Row++) {
    drawFrameVLine(Scr, {Pos.X, Pos.Y + static_cast<int>(Row) + 1}, Size.X,
                   FrameColor);
  }
  drawFrameHLine(Scr, {Pos.X, Pos.Y + static_cast<int>(Size.Y - 2) + 1}, Size.X,
                 FrameColor);
}

} // namespace rogue::ui