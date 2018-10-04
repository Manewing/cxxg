#include <cxxg/Row.h>

#include <stdexcept>

namespace cxxg {

namespace {

static ::std::string getColorStr(Color Cl) {
  return "\033[" + ::std::to_string(Cl.Value) + "m";
}

} // namespace

Color Color::NONE = {0};
Color Color::RED = {31};
Color Color::GREEN = {32};
Color Color::YELLOW = {33};
Color Color::BLUE = {34};

RowAccessor::RowAccessor(Row &Rw, size_t Offset) : Rw(Rw), Offset(Offset) {}

RowAccessor::~RowAccessor() {
  if (Offset < Rw.Buffer.size()) {
    Rw.ColorInfos.insert({Offset, Color::NONE});
  }
}

RowAccessor &RowAccessor::operator=(::std::string const &Str) {
  if (Offset + Str.size() > Rw.Buffer.size()) {
    throw ::std::out_of_range("todo");
  }
  ::std::copy(Str.begin(), Str.end(), Rw.Buffer.begin() + Offset);

  // update offset of accessor by the amount of characters we already wrote
  Offset += Str.size();

  return *this;
}

RowAccessor &RowAccessor::operator<<(Color Cl) {
  if (Offset >= Rw.Buffer.size()) {
    throw ::std::out_of_range("todo");
  }

  Row::ColorInfo CI{Offset, Cl};
  Rw.ColorInfos.erase(CI);
  Rw.ColorInfos.insert(CI);
  return *this;
}

RowAccessor &RowAccessor::operator<<(::std::string const &Str) {
  return this->operator=(Str);
}

Row::Row(size_t Size) { Buffer.resize(Size, ' '); }

void Row::clear() {
  // clear buffer
  ::std::fill(Buffer.begin(), Buffer.end(), ' ');

  // clear color infos
  ColorInfos.clear();
}

RowAccessor Row::operator[](size_t X) { return RowAccessor(*this, X); }

::std::ostream &Row::dump(::std::ostream &Out) const {
  size_t Offset = 0;

  for (auto const &CI : ColorInfos) {
    Out << Buffer.substr(Offset, CI.Offset - Offset) << getColorStr(CI.Cl);
    Offset = CI.Offset;
  }

  Out << Buffer.substr(Offset, Buffer.size() - Offset)
      << getColorStr(Color::NONE);

  return Out;
}

} // namespace cxxg

::std::ostream &operator<<(::std::ostream &Out, ::cxxg::Row const &Rw) {
  return Rw.dump(Out);
}
