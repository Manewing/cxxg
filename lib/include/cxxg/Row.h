#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace cxxg {

class Row;

class RowAccessor {
public:
  RowAccessor(Row &Rw, size_t Offset);
  RowAccessor &operator=(::std::string const &Str);

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
};
}

::std::ostream &operator<<(::std::ostream &Out, ::cxxg::Row const &Rw);
