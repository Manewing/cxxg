#ifndef ROGUE_TILE_H
#define ROGUE_TILE_H

#include <cxxg/Types.h>

struct Tile {
  cxxg::types::ColoredChar T;

  char kind() const { return T.Char; }
  cxxg::types::TermColor color() const { return T.Color; }
};
inline bool operator==(const Tile &A, const Tile &B) noexcept {
  // Ignore coloring when comparing
  return A.kind() == B.kind();
}
inline bool operator!=(const Tile &A, const Tile &B) noexcept {
  return !(A == B);
}

#endif // #ifndef ROGUE_LEVEL_H