#ifndef CXXG_ROW_H
#define CXXG_ROW_H

#include <set>
#include <sstream>
#include <string>

namespace cxxg {

// TODO move?
struct Color {
  static Color NONE;
  static Color RED;
  static Color GREEN;
  static Color YELLOW;
  static Color BLUE;

  int Value;
};

class Row;

class RowAccessor {
public:
  RowAccessor(Row &Rw, size_t Offset);
  virtual ~RowAccessor();

  RowAccessor &operator=(::std::string const &Str);

  RowAccessor &operator<<(Color Cl);
  RowAccessor &operator<<(::std::string const &Str);

  template <typename Type> RowAccessor &operator<<(Type const &T) {
    ::std::stringstream SS;
    SS << T;
    return operator<<(SS.str());
  }

private:
  Row &Rw;
  size_t Offset;
};

class Row {
  friend RowAccessor;

public:
  struct ColorInfo {
    size_t Offset;
    Color Cl;

    bool operator<(ColorInfo const &Other) const {
      return Offset < Other.Offset;
    }
  };

public:
  Row(size_t Size);

  void clear();

  RowAccessor operator[](size_t X);

  ::std::ostream &dump(::std::ostream &Out) const;

private:
  ::std::string Buffer;
  ::std::set<ColorInfo> ColorInfos;
};

} // namespace cxxg

::std::ostream &operator<<(::std::ostream &Out, ::cxxg::Row const &Rw);

#endif // #ifndef CXXG_ROW_H
