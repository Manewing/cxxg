#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/TextBox.h>

namespace rogue::ui {

TextBox::TextBox(cxxg::types::Position Pos, cxxg::types::Size Size,
                 const std::string &Text, cxxg::types::Size Pd)
    : BaseRect(Pos, Size), Wrap(Text, Size.X - Pd.X * 2), Padding(Pd) {}

void TextBox::setText(const std::string &Text) {
  Wrap = WordWrap(Text, Size.X - Padding.X * 2);
}

bool TextBox::handleInput(int Char) {
  switch (Char) {
  case Controls::CloseWindow.Char:
    return false;
  case Controls::MoveDown.Char: {
    const auto NumRows = Size.Y - Padding.Y * 2;
    if (Wrap.getNumLines() >= NumRows && ScrollIdx < Wrap.getNumLines() - 1) {
      ScrollIdx++;
    }
  } break;
  case Controls::MoveUp.Char:
    if (ScrollIdx > 0) {
      ScrollIdx--;
    }
    break;
  default:
    break;
  }
  return true;
}

std::string TextBox::getInteractMsg() const {
  return Controls::Navigate.getInteractMsg();
}

void TextBox::draw(cxxg::Screen &Scr) const {
  BaseRect::draw(Scr);

  static constexpr auto ScrollColor = cxxg::types::RgbColor{90, 130, 175};

  const auto NumTotalLines = Wrap.getNumLines();
  const auto NumRows = Size.Y - Padding.Y * 2;
  for (std::size_t Idx = 0; Idx < NumRows; Idx++) {
    cxxg::types::Position LinePos = {Pos.X, Pos.Y + static_cast<int>(Idx)};
    LinePos += Padding;

    if (ScrollIdx + Idx < NumTotalLines) {
      Scr[LinePos] << Wrap.getLine(ScrollIdx + Idx);
    }
  }

  if (ScrollIdx != 0) {
    cxxg::types::Position LinePos = Pos;
    LinePos += Padding;
    const auto X = LinePos.X + Wrap.getLineWidth() - 1;
    Scr[LinePos.Y][X] = '^';
    Scr[LinePos.Y + 1][X] = '|';
    Scr[LinePos.Y][X] = ScrollColor;
    Scr[LinePos.Y + 1][X] = ScrollColor;
  }

  if (ScrollIdx + NumRows <= NumTotalLines) {
    cxxg::types::Position LinePos = {Pos.X,
                                     Pos.Y + static_cast<int>(NumRows) - 1};
    LinePos += Padding;
    const auto X = LinePos.X + Wrap.getLineWidth() - 1;
    Scr[LinePos.Y - 1][X] = '|';
    Scr[LinePos.Y][X] = 'v';
    Scr[LinePos.Y - 1][X] = ScrollColor;
    Scr[LinePos.Y][X] = ScrollColor;
  }
}

} // namespace rogue::ui