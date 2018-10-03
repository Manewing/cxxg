#include <cxxg/Row.h>

namespace cxxg {

RowAccessor::RowAccessor(Row &Rw, size_t Offset) : Rw(Rw), Offset(Offset) {}

RowAccessor &RowAccessor::operator=(::std::string const &Str) {
  if (Offset + Str.size() >= Rw.Buffer.size()) {
    throw ::std::out_of_range("todo");
  }
  ::std::copy(Str.begin(), Str.end(), Rw.Buffer.begin() + Offset);

  return *this;
}

Row::Row(size_t Size) { Buffer.resize(Size, ' '); }

void Row::clear() { ::std::fill(Buffer.begin(), Buffer.end(), ' '); }

RowAccessor Row::operator[](size_t X) { return RowAccessor(*this, X); }

::std::ostream &Row::dump(::std::ostream &Out) const {
  Out << Buffer;
  return Out;
}
}

::std::ostream &operator<<(::std::ostream &Out, ::cxxg::Row const &Rw) {
  return Rw.dump(Out);
}
