#ifndef CXXG_TYPES_H
#define CXXG_TYPES_H

#include <iosfwd>
#include <string>

namespace cxxg {

namespace types {

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

/// Typesafe struct for holding color information
struct Color {
  static Color NONE;
  static Color RED;
  static Color GREEN;
  static Color YELLOW;
  static Color BLUE;
  static Color GREY;

  /// The color code for terminal, output as "\033$Value[0m"
  int Value;
};

/// Compares if the given two colors are equal, based on the color value.
inline bool operator==(Color const A, Color const B) {
  return A.Value == B.Value;
}

/// Compares if the given two colors are un-equal, based on the color value.
inline bool operator!=(Color const A, Color const B) {
  return A.Value != B.Value;
}

}; // namespace types

}; // namespace cxxg

::std::ostream &operator<<(::std::ostream &Out, ::cxxg::types::Color const Cl);

#endif // #ifndef CXXG_TYPES_H
