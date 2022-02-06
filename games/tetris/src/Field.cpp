#include <Field.h>
#include <algorithm>
#include <cxxg/Screen.h>

Field::Field(unsigned W, unsigned H) : Width(W), Height(H) {
  Blocks.resize(Height, std::vector<Block>(Width));
}

namespace {

void drawRect(cxxg::Screen &Scr, cxxg::types::Position TopLeft,
              cxxg::types::Position BottomRight) {
  Scr[BottomRight.Y][TopLeft.X] = '+';
  Scr[TopLeft.Y][BottomRight.X] = '+';
  Scr[BottomRight.Y][BottomRight.X] = '+';
  Scr[TopLeft.Y][TopLeft.X] = '+';

  for (int X = TopLeft.X + 1; X < BottomRight.X; X++) {
    Scr[TopLeft.Y][X] = '-';
    Scr[BottomRight.Y][X] = '-';
  }
  for (int Y = TopLeft.Y + 1; Y < BottomRight.Y; Y++) {
    Scr[Y][TopLeft.X] = '|';
    Scr[Y][BottomRight.X] = '|';
  }
}

} // namespace

void Field::draw(cxxg::Screen &Scr, cxxg::types::Position Pos) {
  auto TopLeft = Pos + cxxg::types::Position{-1, -1};
  auto BottomRight = Pos + cxxg::types::Position{static_cast<int>(Width),
                                                 static_cast<int>(Height)};
  drawRect(Scr, TopLeft, BottomRight);

  for (unsigned Y = 0; Y < Height; Y++) {
    for (unsigned X = 0; X < Width; X++) {
      const auto &Block = Blocks.at(Y).at(X);
      Scr[Pos.Y + Y][Pos.X + X] = Block.Char;
      Scr[Pos.Y + Y][Pos.X + X] = Block.Color;
    }
  }
}

bool Field::inBounds(cxxg::types::Position Pos) const {
  return !(Pos.X < 0 || Pos.X >= static_cast<int>(Width) || Pos.Y < 0 ||
           Pos.Y >= static_cast<int>(Height));
}

bool Field::inBounds(Tetromino T) const {
  for (auto [Pos, Char] : T) {
    (void)Char;
    if (!inBounds(Pos)) {
      return false;
    }
  }
  return true;
}

bool Field::collides(Tetromino T) const {
  for (auto [Pos, Char] : T) {
    (void)Char;
    if (!inBounds(Pos)) {
      continue;
    }
    if (Blocks.at(Pos.Y).at(Pos.X).Char != ' ') {
      return true;
    }
  }
  return false;
}

bool Field::place(Tetromino T) {
  for (auto [Pos, Char] : T) {
    if (!inBounds(Pos)) {
      continue;
    }
    Blocks[Pos.Y][Pos.X] = {Char, T.getColor()};
  }
  return false;
}

unsigned Field::getNumLines() const { return Blocks.size(); }

bool Field::isLineFull(unsigned Y) const {
  const auto &Line = Blocks.at(Y);
  return std::all_of(Line.begin(), Line.end(),
                     [](const Block &B) { return B.Char != ' '; });
}

std::vector<unsigned> Field::getFullLines() const {
  std::vector<unsigned> FullLines = {};
  for (unsigned Idx = 0; Idx < Blocks.size(); Idx++) {
    if (isLineFull(Idx)) {
      FullLines.push_back(Idx);
    }
  }
  return FullLines;
}

void Field::removeFullLines() {
  BlockMatrix NewBlocks;
  NewBlocks.resize(Blocks.size());

  int Idx = Blocks.size() - 1;
  for (int Y = Blocks.size() - 1; Y >= 0; Y--) {
    if (!isLineFull(Y)) {
      NewBlocks.at(Idx--) = std::move(Blocks.at(Y));
    }
  }
  for (; Idx >= 0; Idx--) {
    NewBlocks.at(Idx).resize(Width, Block());
  }
  Blocks = std::move(NewBlocks);
}

void Field::animateRainbowLine(unsigned Y, unsigned AnimTick) {
  auto &Line = Blocks.at(Y);
  std::fill(Line.begin(), Line.end(), Block{'=', cxxg::types::Color::NONE});
  for (unsigned Idx = 0; Idx < Line.size(); Idx++) {
    unsigned ColorIdx = (Idx + AnimTick) % Tetromino::Colors.size();
    Line[Idx] = {'=', Tetromino::Colors.at(ColorIdx)};
  }
}