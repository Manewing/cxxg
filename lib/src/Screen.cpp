#include <cxxg/Screen.h>
#include <cxxg/Utils.h>

namespace cxxg {

types::Size Screen::getTerminalSize() {
  auto [W, H] = ::cxxg::utils::getTerminalSize();
  return {W, H};
}

Screen::Screen(types::Size Size, ::std::ostream &Out)
    : Out(Out), DummyRow(0), Size(Size) {
  Rows.reserve(Size.Y);
  for (size_t Y = 0; Y < Size.Y; Y++) {
    Rows.push_back(Row(Size.X));
  }
}

Row &Screen::operator[](int Y) {
  // check if out of range
  if (Y < 0 || Y >= static_cast<int>(Rows.size())) {
    return DummyRow;
  }
  return Rows.at(Y);
}

Row const &Screen::operator[](int Y) const {
  // check if out of range
  if (Y < 0 || Y >= static_cast<int>(Rows.size())) {
    return DummyRow;
  }
  return Rows.at(Y);
}

void Screen::setColor(types::Position Top, types::Position Bottom,
                      types::TermColor Cl) {
  for (int Y = Top.Y; Y <= Bottom.Y; Y++) {
    operator[](Y).setColor(Top.X, Bottom.X, Cl);
  }
}

void Screen::update() const {
  std::stringstream SS;
  SS << ClearScreenStr << HideCursorStr;
  for (auto &Row : Rows) {
    SS << Row;
  }
  SS << ShowCursorStr;
  Out << SS.str() << ::std::flush;
}

void Screen::clear() {
  for (auto &Row : Rows) {
    Row.clear();
  }
}

} // namespace cxxg
