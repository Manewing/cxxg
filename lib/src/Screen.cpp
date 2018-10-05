#include <cxxg/Screen.h>

#include <sys/ioctl.h>
#include <unistd.h>

namespace cxxg {

ScreenSize Screen::getTerminalSize() {
  winsize Ws;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &Ws);
  return {Ws.ws_col, Ws.ws_row};
}

Screen::Screen(ScreenSize Size, ::std::ostream &Out) : Out(Out), Size(Size) {
  Rows.reserve(Size.Y);
  for (size_t Y = 0; Y < Size.Y; Y++) {
    Rows.push_back(Row(Size.X));
  }
}

Row &Screen::operator[](size_t Y) { return Rows.at(Y); }

Row const &Screen::operator[](size_t Y) const { return Rows.at(Y); }

void Screen::update() const {
  Out << ClearScreenStr;
  for (auto &Row : Rows) {
    Out << Row;
  }
}

void Screen::clear() {
  for (auto &Row : Rows) {
    Row.clear();
  }
}

} // namespace cxxg
