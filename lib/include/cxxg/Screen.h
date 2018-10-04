#ifndef CXXG_SCREEN_H
#define CXXG_SCREEN_H

#include <iostream>
#include <string>
#include <vector>

#include <cxxg/Row.h>

namespace cxxg {

class Screen {
public:
  static auto constexpr ClearScreenStr = "\e[1;1H\e[2J";

public:
  Screen(size_t SizeX, size_t SizeY, ::std::ostream &Out = ::std::cout);

  Row &operator[](size_t Y);

  Row const &operator[](size_t Y) const;

  void update();

private:
  ::std::ostream &Out;
  ::std::vector<Row> Rows;
};

} // namespace cxxg

#endif // #ifndef CXXG_SCREEN_H
