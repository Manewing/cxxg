#include <cxxg/Screen.h>

#include <sys/ioctl.h>
#include <unistd.h>

namespace cxxg {

ScreenSize Screen::getTerminalSize() {
  winsize Ws;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &Ws);
  return {Ws.ws_col, Ws.ws_row};
}

Screen::Screen(ScreenSize Size, ::std::ostream &Out)
    : Out(Out), DummyRow(0), Size(Size) {
  Rows.reserve(Size.Y);
  for (int Y = 0; Y < Size.Y; Y++) {
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

void Screen::setColor(ScreenSize Top, ScreenSize Bottom, Color Cl) {
  for (int Y = Top.Y; Y <= Bottom.Y; Y++) {
    operator[](Y).setColor(Top.X, Bottom.X, Cl);
  }
}

void Screen::update() const {
  Out << ClearScreenStr << HideCursorStr;
  for (auto &Row : Rows) {
    Out << Row;
  }
  Out << ShowCursorStr << ::std::flush;
}

void Screen::clear() {
  for (auto &Row : Rows) {
    Row.clear();
  }
}

} // namespace cxxg
