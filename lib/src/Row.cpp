#include <cxxg/Row.h>

#include <vector>

namespace cxxg {

namespace {

// TODO move/refactor
static ::std::string const &getColorStr(ColorInfo::ColorCode Color) {
  static ::std::vector<::std::string> Colors = {
      "\033[0m", "\033[31m", "\033[32m", "\033[33m", "\033[34m"};
  return Colors.at(Color);
}

} // namespace

RowAccessor::RowAccessor(Row &Rw, size_t Offset) : Rw(Rw), Offset(Offset) {}

RowAccessor::~RowAccessor() {
  if (Offset < Rw.Buffer.size()) {
    Rw.ColorInfos.insert({Offset, ColorInfo::ColorCode::NONE});
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

RowAccessor &RowAccessor::operator<<(ColorInfo::ColorCode Color) {
  if (Offset >= Rw.Buffer.size()) {
    throw ::std::out_of_range("todo");
  }

  ColorInfo CI{Offset, Color};
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
    Out << Buffer.substr(Offset, CI.Offset - Offset) << getColorStr(CI.Color);
    Offset = CI.Offset;
  }

  Out << Buffer.substr(Offset, Buffer.size() - Offset)
      << getColorStr(ColorInfo::ColorCode::NONE);

  return Out;
}
} // namespace cxxg

::std::ostream &operator<<(::std::ostream &Out, ::cxxg::Row const &Rw) {
  return Rw.dump(Out);
}
