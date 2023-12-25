#ifndef CXXG_SCREEN_H
#define CXXG_SCREEN_H

#include <iostream>
#include <string>
#include <vector>

#include <cxxg/Row.h>
#include <cxxg/Types.h>

namespace cxxg {

class Screen {
public:
  /// String for clearing the terminal
  static auto constexpr ClearScreenStr = "\033[1;1H";

  /// String for turning off cursor in terminal
  static auto constexpr HideCursorStr = "\033[?25l";

  /// String for turning on cursor in terminal
  static auto constexpr ShowCursorStr = "\033[?25h";

public:
  /// Returns the current terminal size
  /// @return The current terminal size in columns and rows
  static types::Size getTerminalSize();

public:
  /// Constructs new screen with given size and output stream
  /// @param[in] Size - Size of the screen in rows and columns
  /// @param[in] Out  - Output stream (default = ::std::cout)
  Screen(types::Size Size, ::std::ostream &Out = ::std::cout);

  /// Returns the size of the screen
  inline types::Size getSize() const { return Size; }

  /// Returns the given row based on the Y-position
  /// @param[in] Y - The column/Y-position
  Row &operator[](int Y);

  /// Returns the given row based on the Y-position
  /// @param[in] Y - The column/Y-position
  Row const &operator[](int Y) const;

  /// Returns a row accessor based on the given position
  /// @param[in] Pos - Position to get the row accessor for
  inline RowAccessor operator[](types::Position const Pos) {
    return operator[](Pos.Y)[Pos.X];
  }

  /// Sets the color for the rectangle defined by [Top, Bottom]
  /// to the given color.
  /// @param[in] Top    - Top corner of the rectangle
  /// @param[in] Bottom - Bottom corner of the rectangle
  /// @param[in] Cl     - Color to set
  void setColor(types::Position Top, types::Position Bottom,
                types::TermColor Cl);

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

  /// Dummy row for out of range accesses
  Row DummyRow;

  /// The screen size
  types::Size Size;
};

} // namespace cxxg

#endif // #ifndef CXXG_SCREEN_H
