#ifndef CXXG_ROW_H
#define CXXG_ROW_H

#include <cxxg/Types.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace cxxg {

// Forward declaration
class Row;

/// Helper class for handling access to a row created from an access
/// with a given offset to a row.
class RowAccessor {
public:
  /// Constructs a new row accessor from the given row and the offset
  /// within the row
  RowAccessor(Row &Rw, int Offset);

  /// Sets color for the current index/offset
  /// @param[in] Cl - Color to set character to
  RowAccessor &operator=(types::TermColor Cl);

  /// Sets character at the current index/offset
  /// @param[in] C - Character to set
  RowAccessor &operator=(char C);

  /// Sets character at the current index/offset
  /// @param[in] C - Character to set
  RowAccessor &operator=(types::ColoredChar C);

  /// Sets new color for current accessor, everything following will
  /// have the given color
  /// @param[in] Cl - The color to set the output to
  RowAccessor &operator<<(types::TermColor Cl);

  /// Outputs the given string to the row, will increase the access offset
  /// by the amount of characters written for the string
  /// @param[in] Str - The string to output
  RowAccessor &operator<<(::std::string const &Str);

  /// Outputs the given string to the row, will increase the access offset
  /// by the amount of characters written for the string
  /// @param[in] Str - The string to output
  RowAccessor &operator<<(::std::string_view const &Str);

  /// Outputs the given type to the row, first the the type will be converted
  /// to string via a string stream, the resulting string will the be output
  /// @param[in] T - The variable with type 'T' to output
  template <typename Type> RowAccessor &operator<<(Type const &T) {
    ::std::stringstream SS;
    SS << T;
    return operator<<(SS.str());
  }

private:
  /// The row to access
  Row &Rw;

  /// The offset to the row, changes after modification
  int Offset;

  /// The current color for output
  types::TermColor CurrentColor;
};

/// Class for representing a row in the screen (terminal), provides
/// access via array operator.
class Row {
  /// Allow access to private members for row accessor
  friend RowAccessor;

public:
  /// Creates a new row with given size
  /// @param[in] Size - The size of the row
  Row(size_t Size);

  /// Clears the row and resets color information
  void clear();

  /// Returns the internal buffer of the row
  ::std::string const &getBuffer() const;

  /// Returns the internal color information of the row
  ::std::vector<types::TermColor> const &getColorInfo() const;

  /// Provides access to the row with a given X offset
  /// @param[in] X - The offset for the access
  RowAccessor operator[](int X);

  /// Sets the color for the interval [StartX, EndX] to the given
  /// color.
  /// @param[in] StartX - Start of interval (included)
  /// @param[in] EndX   - End of internval (included)
  /// @param[in] Cl     - The color to set
  void setColor(int StartX, int EndX, types::TermColor Cl);

  /// Dumps the row to given stream
  /// @param[in/out] Out - The output stream to dump the row to
  /// @returns The modified output stream
  ::std::ostream &dump(::std::ostream &Out) const;

private:
  /// The internal buffer of the row
  ::std::string Buffer;

  /// Color information of the row
  ::std::vector<types::TermColor> ColorInfo;
};

} // namespace cxxg

/// Outstream operator of Row for convenience
/// @param[in/out] Out - The output stream to write row content to
/// @param[in]     Rw  - The row to output
/// @returns The modified output stream
::std::ostream &operator<<(::std::ostream &Out, ::cxxg::Row const &Rw);

#endif // #ifndef CXXG_ROW_H
