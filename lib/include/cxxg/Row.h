#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

namespace cxxg {

struct ColorInfo {
  typedef enum { NONE, RED, GREEN, YELLOW, BLUE } ColorCode;

  size_t Offset;
  ColorCode Color;

  bool operator<(ColorInfo const &Other) const { return Offset < Other.Offset; }
};

class Row;

class RowAccessor {
public:
  RowAccessor(Row &Rw, size_t Offset);
  virtual ~RowAccessor();

  RowAccessor &operator=(::std::string const &Str);

  RowAccessor &operator<<(ColorInfo::ColorCode Color);
  RowAccessor &operator<<(::std::string const &Str);

private:
  Row &Rw;
  size_t Offset;
};

class Row {
  friend RowAccessor;

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
