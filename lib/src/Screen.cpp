#include <cxxg/Screen.h>

namespace cxxg {

Screen::Screen(ScreenSize Size, ::std::ostream &Out) : Out(Out), Size(Size) {
  Rows.reserve(Size.Y);
  for (size_t Y = 0; Y < Size.Y; Y++) {
    Rows.push_back(Row(Size.X));
  }
}

Row &Screen::operator[](size_t Y) { return Rows.at(Y); }

Row const &Screen::operator[](size_t Y) const { return Rows.at(Y); }

void Screen::update() {
  Out << ClearScreenStr;
  for (auto &Row : Rows) {
    Out << Row;
    Row.clear();
  }
}
} // namespace cxxg
