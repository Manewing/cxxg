#include <cxxg/Types.h>
#include <cxxg/Row.h>

namespace cxxg {

RowAccessor::RowAccessor(Row &Rw, int Offset)
    : Rw(Rw), Offset(Offset), CurrentColor(types::Color::NONE) {}

RowAccessor::RowAccessor(RowAccessor &&RA)
    : Rw(RA.Rw), Offset(RA.Offset), CurrentColor(RA.CurrentColor),
      SS(std::move(RA.SS)) {
  RA.Valid = false;
}

RowAccessor::~RowAccessor() { flushBuffer(); }

RowAccessor &RowAccessor::operator=(types::TermColor Cl) {
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

RowAccessor &RowAccessor::operator=(types::ColoredChar C) {
  *this = C.Color;
  *this = C.Char;
  return *this;
}

RowAccessor &RowAccessor::operator<<(types::TermColor Cl) {
  flushBuffer();
  CurrentColor = Cl;
  return *this;
}

RowAccessor &RowAccessor::operator<<(const Row &OtherRw) {
  for (size_t L = 0; L < OtherRw.Buffer.size(); L++) {
    if (CurrentColor != OtherRw.ColorInfo.at(L)) {
      *this << OtherRw.ColorInfo.at(L);
    }
    *this << OtherRw.Buffer.at(L);
  }
  *this << types::Color::NONE;
  return *this;
}

RowAccessor &RowAccessor::operator<<(const RWidth &W) {
  MaxWidth = W.Width;
  return *this;
}

void RowAccessor::flushBuffer() {
  if (!Valid) {
    return;
  }
  auto Str = SS.str();
  if (MaxWidth && NumCharactersWritten + Str.size() > *MaxWidth) {
    auto Length = *MaxWidth - NumCharactersWritten;
    if (Length > 0) {
      Str = Str.substr(0, Length);
    } else {
      Str = "";
    }
  }
  output(Str);
  SS.str("");
  SS.clear();
}

RowAccessor &RowAccessor::width(std::size_t Width) {
  this->MaxWidth = Width;
  return *this;
}

void RowAccessor::output(::std::string const &Str) {
  if (Str.empty()) {
    return;
  }

  // check if the access is out of range, if so ignore it
  if (Rw.Buffer.size() == 0 || Offset >= static_cast<int>(Rw.Buffer.size()) ||
      Offset + static_cast<int>(Str.size()) <= 0) {
    Offset += Str.size();
    return;
  }

  // get row buffer and color info
  auto &Buffer = Rw.Buffer;
  auto &ColorInfo = Rw.ColorInfo;

  // keep track of how much we moved offset
  size_t Count;

  // FIXME four cases?? 4. is before start and after end
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

  Offset += Str.size();
  NumCharactersWritten += Str.size();
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

::std::vector<types::TermColor> const &Row::getColorInfo() const {
  return ColorInfo;
}

RowAccessor Row::operator[](int X) { return RowAccessor(*this, X); }

void Row::setColor(int StartX, int EndX, types::TermColor Cl) {
  int Start = ::std::max(StartX, 0);
  int End = ::std::max(0, ::std::min(EndX, static_cast<int>(ColorInfo.size())));

  if (End > Start) {
    ::std::fill(ColorInfo.begin() + Start, ColorInfo.begin() + End, Cl);
  }
}

::std::ostream &Row::dump(::std::ostream &Out) const {
  types::TermColor LastColor = types::Color::NONE;

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
