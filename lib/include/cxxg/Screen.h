#ifndef CXXG_SCREEN_H
#define CXXG_SCREEN_H

#include <iostream>
#include <string>
#include <vector>

#include <cxxg/Row.h>

namespace cxxg {

struct ScreenSize {
  size_t X;
  size_t Y;
};

class Screen {
public:
  static auto constexpr ClearScreenStr = "\e[1;1H\e[2J";

public:
  Screen(ScreenSize Size, ::std::ostream &Out = ::std::cout);

  ScreenSize getSize() const { return Size; }

  Row &operator[](size_t Y);

  Row const &operator[](size_t Y) const;

  void update();

private:
  ::std::ostream &Out;
  ::std::vector<Row> Rows;
  ScreenSize Size;
};

} // namespace cxxg

#endif // #ifndef CXXG_SCREEN_H
