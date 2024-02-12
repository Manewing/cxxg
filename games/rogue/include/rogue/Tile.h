#ifndef ROGUE_TILE_H
#define ROGUE_TILE_H

#include <cxxg/Types.h>
#include <iosfwd>

// Custom serialization functions for cxxg types
namespace cxxg::types {

template <class Archive> void serialize(Archive &Ar, FontStyle &FS) {
  // cereal has problems with bitfields, so we have to serialize them manually
  unsigned Italic = FS.Italic;
  unsigned Bold = FS.Bold;
  unsigned Underline = FS.Underline;
  unsigned Strikethrough = FS.Strikethrough;
  Ar(Italic, Bold, Underline, Strikethrough);
  FS = FontStyle{Italic, Bold, Underline, Strikethrough};
}

template <class Archive> void serialize(Archive &Ar, RgbColor &RC) {
  Ar(RC.R, RC.G, RC.B, RC.HasBackground, RC.BgR, RC.BgG, RC.BgB, RC.FS);
}

template <class Archive> void serialize(Archive &, NoColor &) {}

template <class Archive> void serialize(Archive &Ar, DefaultColor &DC) {
  Ar(DC.FS);
}

template <class Archive> void serialize(Archive &Ar, ColoredChar &CC) {
  Ar(CC.Char, CC.Color);
}

} // namespace cxxg::types

namespace rogue {

struct Tile {
  cxxg::types::ColoredChar T;

  char &kind() { return T.Char; }
  cxxg::types::TermColor &color() { return T.Color; }
  const char &kind() const { return T.Char; }
  const cxxg::types::TermColor &color() const { return T.Color; }

  /// Higher z-index means closer to user (overlaps)
  int ZIndex = 0;

  template <class Archive> void serialize(Archive &Ar) { Ar(T, ZIndex); }
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