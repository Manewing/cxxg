#ifndef ROGUE_TILE_H
#define ROGUE_TILE_H

#include <cxxg/Types.h>
#include <iosfwd>

namespace rogue {

struct Tile {
  cxxg::types::ColoredChar T;

  char &kind() { return T.Char; }
  cxxg::types::TermColor &color() { return T.Color; }
  const char &kind() const { return T.Char; }
  const cxxg::types::TermColor &color() const { return T.Color; }

  // FIXME this is duplicated in TileComp
  int ZIndex = 0;
};
inline bool operator==(const Tile &A, const Tile &B) noexcept {
  // Ignore coloring when comparing
  return A.kind() == B.kind();
}
inline bool operator!=(const Tile &A, const Tile &B) noexcept {
  return !(A == B);
}

inline std::ostream &operator<<(std::ostream &Out, Tile &T) {
  Out << T.T;
  return Out;
}

} // namespace rogue

#endif // #ifndef ROGUE_LEVEL_H