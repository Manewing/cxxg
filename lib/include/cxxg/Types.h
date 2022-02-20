#ifndef CXXG_TYPES_H
#define CXXG_TYPES_H

#include <iosfwd>
#include <string>
#include <tuple>
#include <variant>

namespace cxxg {

namespace types {

template <class... Ts> struct Overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;

/// Position in rows and columns
struct Position {
  /// X-Position (Column)
  int X;

  /// Y-Position (Row)
  int Y;

  /// Adds other position to this
  inline Position &operator+=(Position const P) {
    X += P.X;
    Y += P.Y;
    return *this;
  }

  /// Subtracts other position from this
  inline Position &operator-=(Position const P) {
    X -= P.X;
    Y -= P.Y;
    return *this;
  }
};

/// Adds two positions together
inline Position operator+(Position const A, Position const B) {
  return {A.X + B.X, A.Y + B.Y};
}

/// Subtracts two positions from each other
inline Position operator-(Position const A, Position const B) {
  return {A.X - B.X, A.Y - B.Y};
}

inline bool operator==(const Position &Lhs, const Position &Rhs) {
  return Lhs.X == Rhs.X && Lhs.Y == Rhs.Y;
}
inline bool operator!=(const Position &Lhs, const Position &Rhs) {
  return !(Lhs == Rhs);
}

/// Size in rows and columns
struct Size {
  /// Size X, Columns
  size_t X;

  /// Size Y, Rows
  size_t Y;

  // allow implicit conversion to position
  operator Position() const {
    return {static_cast<int>(X), static_cast<int>(Y)};
  }

  /// Adds other size to this
  inline Size &operator+=(Size const S) {
    X += S.X;
    Y += S.Y;
    return *this;
  }
};

/// Adds to sizes together
inline Size operator+(Size const A, Size const B) {
  return {A.X + B.X, A.Y + B.Y};
}

struct NoColor {};
inline bool operator==(const NoColor &, const NoColor &) noexcept {
  return true;
}
inline bool operator!=(const NoColor &, const NoColor &) noexcept {
  return false;
}
std::ostream &operator<<(std::ostream &Out, const NoColor &NC);

struct DefaultColor {};
inline bool operator==(const DefaultColor &, const DefaultColor &) noexcept {
  return true;
}
inline bool operator!=(const DefaultColor &, const DefaultColor &) noexcept {
  return false;
}
std::ostream &operator<<(std::ostream &Out, const DefaultColor &DC);

struct RgbColor {
  static RgbColor getHeatMapColor(float Min, float Max, float Value);

  uint8_t R = 0;
  uint8_t G = 0;
  uint8_t B = 0;
  bool HasBackground = false;
  uint8_t BgR = 0;
  uint8_t BgG = 0;
  uint8_t BgB = 0;
};
inline bool operator==(const RgbColor &Lhs, const RgbColor &Rhs) noexcept {
  return std::tie(Lhs.R, Lhs.G, Lhs.B, Lhs.HasBackground, Lhs.BgR, Lhs.BgG,
                  Lhs.BgB) == std::tie(Rhs.R, Rhs.G, Rhs.B, Rhs.HasBackground,
                                       Rhs.BgR, Rhs.BgG, Rhs.BgB);
}
inline bool operator!=(const RgbColor &Lhs, const RgbColor &Rhs) noexcept {
  return !(Lhs == Rhs);
}
std::ostream &operator<<(std::ostream &Out, const RgbColor &Color);

using TermColor = std::variant<NoColor, DefaultColor, RgbColor>;
std::ostream &operator<<(std::ostream &Out, const TermColor &TC);

namespace Color {
static constexpr TermColor NONE = DefaultColor{};
static constexpr TermColor RED = RgbColor{255, 25, 25};
static constexpr TermColor GREEN = RgbColor{25, 255, 25};
static constexpr TermColor YELLOW = RgbColor{255, 255, 25};
static constexpr TermColor BLUE = RgbColor{80, 80, 255};
static constexpr TermColor GREY = RgbColor{100, 100, 100};
}; // namespace Color

struct ColoredChar {
  char Char = 0;
  TermColor Color = NoColor{};

  constexpr ColoredChar() = default;
  constexpr ColoredChar(char Char) : Char(Char) {}
  constexpr ColoredChar(char Char, TermColor Color)
      : Char(Char), Color(Color) {}
  ColoredChar &operator=(char Char);
  ColoredChar &operator=(const TermColor &Color);
};
inline bool operator==(const ColoredChar &Lhs,
                       const ColoredChar &Rhs) noexcept {
  return Lhs.Char == Rhs.Char && Lhs.Color == Rhs.Color;
}
inline bool operator!=(const ColoredChar &Lhs,
                       const ColoredChar &Rhs) noexcept {
  return !(Lhs == Rhs);
}
std::ostream &operator<<(std::ostream &Out, const ColoredChar &Char);

}; // namespace types

}; // namespace cxxg

#endif // #ifndef CXXG_TYPES_H
