#include <cxxg/Screen.h>
#include <rogue/UI/Widget.h>

namespace rogue::ui {

BaseRect::BaseRect(cxxg::types::Position Pos, cxxg::types::Size Size)
    : Pos(Pos), Size(Size) {}

void BaseRect::draw(cxxg::Screen &Scr) const {
  auto FillStr = std::string(Size.X, ' ');
  for (unsigned Row = 0; Row < Size.Y; Row++) {
    Scr[Pos.Y + Row][Pos.X] << FillStr;
  }
}

} // namespace rogue::ui