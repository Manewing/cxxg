#ifndef CXXG_SCREEN_H
#define CXXG_SCREEN_H

#include <iostream>
#include <string>
#include <vector>

#include <cxxg/Row.h>

namespace cxxg {

/// Screen size in rows and columns
struct ScreenSize {
  /// Size X, Colums
  size_t X;

  /// Size Y, Rows
  size_t Y;
};

class Screen {
public:
  /// String for clearing the terminal
  static auto constexpr ClearScreenStr = "\e[1;1H\e[2J";

public:
  /// Returns the current terminal size
  /// @return The current terminal size in columns and rows
  static ScreenSize getTerminalSize();

public:
  /// Constructs new screen with given size and output stream
  /// @param[in] Size - Size of the screen in rows and columns
  /// @param[in] Out  - Output stream (default = ::std::cout)
  Screen(ScreenSize Size, ::std::ostream &Out = ::std::cout);

  /// Returns the size of the screen
  ScreenSize getSize() const { return Size; }

  /// Returns the given row based on the Y-position
  /// @param[in] Y - The column/Y-position
  Row &operator[](size_t Y);

  /// Returns the given row based on the Y-position
  /// @param[in] Y - The column/Y-position
  Row const &operator[](size_t Y) const;

  /// Updates the screen by writing buffer to output stream
  void update() const;

  /// Clears internal buffers. Note for emptying the screen an update
  /// needs to follow.
  void clear();

private:
  /// The output stream to write to
  ::std::ostream &Out;

  /// Rows of the screen
  ::std::vector<Row> Rows;

  /// The screen size
  ScreenSize Size;
};

} // namespace cxxg

#endif // #ifndef CXXG_SCREEN_H
