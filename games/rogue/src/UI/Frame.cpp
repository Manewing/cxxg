#include <rogue/UI/Frame.h>
#include <cxxg/Screen.h>

namespace rogue::ui {

void Frame::drawFrameHeader(cxxg::Screen &Scr, cxxg::types::Position Pos,
                            std::string_view Header, unsigned Width) {
  drawFrameHLine(Scr, Pos, Width);
  unsigned HdrOffset = (Width - Header.size()) / 2 - 1;
  Scr[Pos.Y][Pos.X + HdrOffset] << "[" << Header << "]";
}

void Frame::drawFrameHLine(cxxg::Screen &Scr, cxxg::types::Position Pos,
                           unsigned Width) {
  Scr[Pos.Y][Pos.X] << "+" << std::string(Width - 2, '-') << "+";
}

void Frame::drawFrameVLine(cxxg::Screen &Scr, cxxg::types::Position Pos,
                           unsigned Width) {
  Scr[Pos.Y][Pos.X] = '|';
  Scr[Pos.Y][Pos.X + Width - 1] = '|';
}

Frame::Frame(std::shared_ptr<Widget> Comp, cxxg::types::Position Pos,
             cxxg::types::Size Size, std::string Header)
    : Decorator(std::move(Comp)), Pos(Pos), Size(Size),
      Header(std::move(Header)) {}

void Frame::draw(cxxg::Screen &Scr) const {
  Decorator::draw(Scr);
  // Draw frame
  if (Header.empty()) {
    drawFrameHLine(Scr, {Pos.X, Pos.Y}, Size.X);
  } else {
    drawFrameHeader(Scr, {Pos.X, Pos.Y}, Header, Size.X);
  }
  for (unsigned Row = 0; Row < Size.Y - 2; Row++) {
    drawFrameVLine(Scr, {Pos.X, Pos.Y + static_cast<int>(Row) + 1}, Size.X);
  }
  drawFrameHLine(Scr, {Pos.X, Pos.Y + static_cast<int>(Size.Y - 2) + 1},
                 Size.X);
}

} // namespace rogue::ui