#include <Tetromino.h>
#include <array>
#include <cxxg/Screen.h>

namespace Shape {

static std::vector<std::string> LL = {
    ".##.",
    ".#..",
    ".#..",
    "....",
};

static std::vector<std::string> RL = {
    ".##.",
    "..#.",
    "..#.",
    "....",
};

static std::vector<std::string> Square = {
    "....",
    ".##.",
    ".##.",
    "....",
};

static std::vector<std::string> Line = {
    "....",
    "....",
    "####",
    "....",
};

static std::vector<std::string> ZR = {
    "..#.",
    ".##.",
    ".#..",
    "....",
};

static std::vector<std::string> ZL = {
    ".#..",
    ".##.",
    "..#.",
    "....",
};

static std::vector<std::string> T = {
    "..#.",
    ".##.",
    "..#.",
    "....",
};

static constexpr std::array Shapes = {
    &LL,
    &RL,
    &Square,
    &Line,
    &ZR,
    &ZL,
    &T
};

}; // namespace Shape

const std::vector<cxxg::types::TermColor> Tetromino::Colors = {
    cxxg::types::Color::RED,
    cxxg::types::Color::BLUE,
    cxxg::types::Color::YELLOW,
    cxxg::types::Color::GREEN,
};

Tetromino::Tetromino(cxxg::types::TermColor Color,
                     const std::vector<std::string> &Shp)
    : Color(Color), Shape(&Shp) {
  // TODO check size
}

const unsigned Tetromino::NumShapes = Shape::Shapes.size();

void Tetromino::draw(cxxg::Screen &Scr, cxxg::types::Position Offset) {
  for (auto [Pos, Char] : *this) {
    Scr[Pos.Y + Offset.Y][Pos.X + Offset.X] = Char;
    Scr[Pos.Y + Offset.Y][Pos.X + Offset.X] = Color;
  }
}

void Tetromino::rotate(int Times) {
  const auto Size = Rotations.size();

  // Get the positive remainder of the modulo
  unsigned Index = ((Times % Size) + Size) % Size;
  Index = (Index + static_cast<unsigned>(Rot)) % Size;

  this->Rot = Rotations.at(Index);
}

cxxg::types::TermColor Tetromino::getColor() const { return Color; }

void Tetromino::setRotation(Rotation Rot) { this->Rot = Rot; }

Tetromino::Rotation Tetromino::getRotation() const { return Rot; }

void Tetromino::setPosition(cxxg::types::Position P) { Pos = P; }

cxxg::types::Position Tetromino::getPosition() const { return Pos; }

const std::vector<std::string> &Tetromino::getShape() const { return *Shape; }

Tetromino Tetromino::create(cxxg::types::TermColor Color, unsigned ShapeIdx) {
  return Tetromino(Color, *Shape::Shapes.at(ShapeIdx));
}

cxxg::types::Position Tetromino::rotate(cxxg::types::Position Pos,
                                        Rotation Rot) {
  switch (Rot) {
  case Rotation::Up: // 0°
    return Pos;
  case Rotation::Right: // 90*
    return {Pos.Y, 3 - Pos.X};
  case Rotation::Down: // 180°
    return {3 - Pos.X, 3 - Pos.Y};
  case Rotation::Left: // 270°
    return {3 - Pos.Y, Pos.X};
  }
  return Pos;
}