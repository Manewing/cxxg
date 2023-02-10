#ifndef FIELD_H
#define FIELD_H

#include <vector>

#include <cxxg/Types.h>

#include <Tetromino.h>

namespace cxxg {
class Screen;
}

struct Block {
  char Char = ' ';
  cxxg::types::TermColor Color = cxxg::types::Color::NONE;
};

class Field {
public:
  using BlockMatrix = std::vector<std::vector<Block>>;

public:
  Field() = delete;
  Field(unsigned Width, unsigned Heigh);

  void draw(cxxg::Screen &Scr, cxxg::types::Position Pos = {0, 0});

  bool inBounds(cxxg::types::Position Pos) const;
  bool inBounds(Tetromino T) const;
  bool collides(Tetromino T) const;
  bool place(Tetromino T);

  unsigned getNumLines() const;
  bool isLineFull(unsigned Y) const;
  std::vector<unsigned> getFullLines() const;

  void removeFullLines();

  void animateRainbowLine(unsigned Y, unsigned AnimTick);

private:
  unsigned Width = 0;
  unsigned Height = 0;
  BlockMatrix Blocks;
};

#endif // #ifndef FIELD_H