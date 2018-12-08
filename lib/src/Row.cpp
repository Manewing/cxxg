#include <cxxg/Row.h>

#include <stdexcept>

namespace cxxg {

RowAccessor::RowAccessor(Row &Rw, int Offset)
    : Rw(Rw), Offset(Offset), CurrentColor(types::Color::NONE) {}

RowAccessor &RowAccessor::operator=(types::Color Cl) {
  // check if the access is out of range, if so ignore it
  if (Offset >= static_cast<int>(Rw.ColorInfo.size()) || Offset < 0) {
    return *this;
  }
  Rw.ColorInfo[Offset] = Cl;
  return *this;
}

RowAccessor &RowAccessor::operator=(char C) {
  // check if the access is out of range, if so ignore it
  if (Offset >= static_cast<int>(Rw.Buffer.size()) || Offset < 0) {
    return *this;
  }
  Rw.Buffer[Offset] = C;
  return *this;
}

RowAccessor &RowAccessor::operator<<(types::Color Cl) {
  CurrentColor = Cl;
  return *this;
}

RowAccessor &RowAccessor::operator<<(::std::string const &Str) {
  // check if the access is out of range, if so ignore it
  if (Offset >= static_cast<int>(Rw.Buffer.size()) ||
      Offset + Str.size() <= 0) {
    return *this;
  }

  // get row buffer and color info
  auto &Buffer = Rw.Buffer;
  auto &ColorInfo = Rw.ColorInfo;

  // keep track of how much we moved offset
  size_t Count;

  // We have got three cases
  if (Offset < 0) {
    // First case: Writing before row with part of string overlapping into
    // the row, we need to clip string at the beginning
    Count = Str.size() + Offset;
    ::std::copy(Str.begin() + (-Offset), Str.end(), Buffer.begin());
    ::std::fill(ColorInfo.begin(), ColorInfo.begin() + Count, CurrentColor);
  } else if (Str.size() + Offset <= Buffer.size()) {
    // Second case: Writing inside of the row, no clipping needed just copy
    // the complete string to buffer with offset
    Count = Str.size();
    ::std::copy(Str.begin(), Str.end(), Buffer.begin() + Offset);
    ::std::fill(ColorInfo.begin() + Offset, ColorInfo.begin() + Offset + Count,
                CurrentColor);
  } else {
    // Third case: Writing inside of the buffer with part of string exceeding
    // the end of the buffer, we need to clip string at the end
    Count = Buffer.size() - Offset;
    ::std::copy(Str.begin(), Str.begin() + Count, Buffer.begin() + Offset);
    ::std::fill(ColorInfo.begin() + Offset, ColorInfo.begin() + Offset + Count,
                CurrentColor);
  }

  // update offset of accessor by the amount of characters we already wrote
  Offset += Count;

  return *this;
}

Row::Row(size_t Size) {
  Buffer.resize(Size, ' ');
  ColorInfo.resize(Size, types::Color::NONE);
}

void Row::clear() {
  // clear buffer
  ::std::fill(Buffer.begin(), Buffer.end(), ' ');

  // clear color infos
  ::std::fill(ColorInfo.begin(), ColorInfo.end(), types::Color::NONE);
}

::std::string const &Row::getBuffer() const { return Buffer; }

::std::vector<types::Color> const &Row::getColorInfo() const {
  return ColorInfo;
}

RowAccessor Row::operator[](int X) { return RowAccessor(*this, X); }

void Row::setColor(int StartX, int EndX, types::Color Cl) {
  int Start = ::std::max(StartX, 0);
  int End = ::std::max(0, ::std::min(EndX, static_cast<int>(ColorInfo.size())));

  if (End > Start) {
    ::std::fill(ColorInfo.begin() + Start, ColorInfo.begin() + End, Cl);
  }
}

::std::ostream &Row::dump(::std::ostream &Out) const {
  types::Color LastColor = types::Color::NONE;

  for (size_t L = 0; L < Buffer.size(); L++) {
    if (LastColor != ColorInfo.at(L)) {
      Out << ColorInfo.at(L);
      LastColor = ColorInfo.at(L);
    }
    Out << Buffer.at(L);
  }
  Out << types::Color::NONE;

  return Out;
}

} // namespace cxxg

::std::ostream &operator<<(::std::ostream &Out, ::cxxg::Row const &Rw) {
  return Rw.dump(Out);
}
