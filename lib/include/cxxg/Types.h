#ifndef CXXG_TYPES_H
#define CXXG_TYPES_H

#include <iosfwd>
#include <stdint.h>
#include <string>
#include <tuple>
#include <variant>

namespace cxxg::types {

template <class... Ts> struct Overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;

template <typename T, typename... Ts>
using EnableIfOfType =
    std::enable_if<std::disjunction<std::is_same<T, Ts>...>::value>;

/// Position in rows and columns
struct Position {
  using ValueType = int;

  /// X-Position (Column)
  ValueType X;

  /// Y-Position (Row)
  ValueType Y;

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
  using ValueType = std::size_t;

  /// Size X, Columns
  ValueType X;

  /// Size Y, Rows
  ValueType Y;

  /// Creates a size from a position, negative values are clamped to 0
  static constexpr Size clamp(const Position &P) {
    return {static_cast<ValueType>(P.X < 0 ? 0 : P.X),
            static_cast<ValueType>(P.Y < 0 ? 0 : P.Y)};
  }

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

/// Scales a size by a factor
inline Size operator*(Size const S, Size::ValueType Factor) {
  return {S.X * Factor, S.Y * Factor};
}

/// Divides a size by a factor
inline Size operator/(Size const S, Size::ValueType Factor) {
  return {S.X / Factor, S.Y / Factor};
}

struct FontStyle {
  unsigned Italic : 1;
  unsigned Bold : 1;
  unsigned Underline : 1;
  unsigned Strikethrough : 1;
};
inline bool operator==(const FontStyle &Lhs, const FontStyle &Rhs) noexcept {
  return std::tie(Lhs.Italic, Lhs.Bold, Lhs.Underline, Lhs.Strikethrough) ==
         std::tie(Rhs.Italic, Rhs.Bold, Rhs.Underline, Rhs.Strikethrough);
}
inline bool operator!=(const FontStyle &Lhs, const FontStyle &Rhs) noexcept {
  return !(Lhs == Rhs);
}
std::ostream &operator<<(std::ostream &Out, const FontStyle &FS);

struct NoColor {};
inline bool operator==(const NoColor &, const NoColor &) noexcept {
  return true;
}
inline bool operator!=(const NoColor &, const NoColor &) noexcept {
  return false;
}
std::ostream &operator<<(std::ostream &Out, const NoColor &NC);

struct DefaultColor {
  FontStyle FS{};

  constexpr auto italic(bool Enabled = true) const {
    auto Copy = *this;
    Copy.FS.Italic = Enabled ? 1 : 0;
    return Copy;
  }

  constexpr auto bold(bool Enabled = true) const {
    auto Copy = *this;
    Copy.FS.Bold = Enabled ? 1 : 0;
    return Copy;
  }

  constexpr auto underline(bool Enabled = true) const {
    auto Copy = *this;
    Copy.FS.Underline = Enabled ? 1 : 0;
    return Copy;
  }

  constexpr auto striketrhough(bool Enabled = true) const {
    auto Copy = *this;
    Copy.FS.Strikethrough = Enabled ? 1 : 0;
    return Copy;
  }
};
inline bool operator==(const DefaultColor &Lhs,
                       const DefaultColor &Rhs) noexcept {
  return Lhs.FS == Rhs.FS;
}
inline bool operator!=(const DefaultColor &Lhs,
                       const DefaultColor &Rhs) noexcept {
  return !(Lhs.FS == Rhs.FS);
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
  FontStyle FS{};

  constexpr auto italic(bool Enabled = true) const {
    auto Copy = *this;
    Copy.FS.Italic = Enabled ? 1 : 0;
    return Copy;
  }

  constexpr auto bold(bool Enabled = true) const {
    auto Copy = *this;
    Copy.FS.Bold = Enabled ? 1 : 0;
    return Copy;
  }

  constexpr auto underline(bool Enabled = true) const {
    auto Copy = *this;
    Copy.FS.Underline = Enabled ? 1 : 0;
    return Copy;
  }

  constexpr auto striketrhough(bool Enabled = true) const {
    auto Copy = *this;
    Copy.FS.Strikethrough = Enabled ? 1 : 0;
    return Copy;
  }
};
inline bool operator==(const RgbColor &Lhs, const RgbColor &Rhs) noexcept {
  return std::tie(Lhs.R, Lhs.G, Lhs.B, Lhs.HasBackground, Lhs.BgR, Lhs.BgG,
                  Lhs.BgB, Lhs.FS) == std::tie(Rhs.R, Rhs.G, Rhs.B,
                                               Rhs.HasBackground, Rhs.BgR,
                                               Rhs.BgG, Rhs.BgB, Rhs.FS);
}
inline bool operator!=(const RgbColor &Lhs, const RgbColor &Rhs) noexcept {
  return !(Lhs == Rhs);
}
std::ostream &operator<<(std::ostream &Out, const RgbColor &Color);

using TermColor = std::variant<NoColor, DefaultColor, RgbColor>;
template <typename T>
using IsTermColor = EnableIfOfType<T, NoColor, DefaultColor, RgbColor>;
std::ostream &operator<<(std::ostream &Out, const TermColor &TC);

template <typename T, typename IsTermColor<T>::type * = nullptr>
inline bool operator==(const TermColor &Lhs, const T &Rhs) noexcept {
  return std::holds_alternative<T>(Lhs) && std::get<T>(Lhs) == Rhs;
}
template <typename T, typename IsTermColor<T>::type * = nullptr>
inline bool operator==(const T &Lhs, const TermColor &Rhs) noexcept {
  return (Rhs == Lhs);
}
template <typename T, typename IsTermColor<T>::type * = nullptr>
inline bool operator!=(const TermColor &Lhs, const T &Rhs) noexcept {
  return !(Lhs == Rhs);
}
template <typename T, typename IsTermColor<T>::type * = nullptr>
inline bool operator!=(const T &Lhs, const TermColor &Rhs) noexcept {
  return (Rhs != Lhs);
}

namespace Color {
static constexpr auto NONE = DefaultColor{};
static constexpr auto RED = RgbColor{255, 25, 25};
static constexpr auto GREEN = RgbColor{25, 255, 25};
static constexpr auto YELLOW = RgbColor{255, 255, 25};
static constexpr auto BLUE = RgbColor{80, 80, 255};
static constexpr auto GREY = RgbColor{100, 100, 100};
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

} // namespace cxxg::types

#endif // #ifndef CXXG_TYPES_H
