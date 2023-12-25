#include <cxxg/Screen.h>
#include <cxxg/Utils.h>

namespace cxxg {

types::Size Screen::getTerminalSize() {
  return ::cxxg::utils::getTerminalSize();
}

Screen::Screen(types::Size S, ::std::ostream &Out)
    : Out(Out), DummyRow(0), Size({0, 0}) {
  resize(S);

  utils::registerWindowResizeHandler(
      [this](types::Size NewSize) { resize(NewSize); });
}

void Screen::resize(types::Size S) {
  if (S == Size) {
    return;
  }
  Size = S;
  Rows.clear();
  for (size_t Y = 0; Y < Size.Y; Y++) {
    Rows.push_back(Row(Size.X));
  }
  if (ResizeHandler) {
    ResizeHandler(*this);
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

void Screen::registerResizeHandler(
    ::std::function<void(const Screen &)> const &Handler) {
  ResizeHandler = Handler;
}

} // namespace cxxg
