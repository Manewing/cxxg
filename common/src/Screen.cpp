#include <cxxg/Screen.h>

namespace cxxg {

Screen::Screen(size_t SizeX, size_t SizeY, ::std::ostream &Out) : Out(Out) {
  Rows.reserve(SizeY);
  for (size_t Y = 0; Y < SizeY; Y++) {
    Rows.push_back(Row(SizeX));
  }
}

Row &Screen::operator[](size_t Y) { return Rows.at(Y); }

Row const &Screen::operator[](size_t Y) const { return Rows.at(Y); }

void Screen::update() const {
  Out << ClearScreenStr;
  for (auto const &Row : Rows) {
    Out << Row;
  }
}
}
