#include <cxxg/Screen.h>

#include <sstream>

int main() {
  ::std::stringstream SS;
  ::std::stringstream Ref;

  Ref << ::cxxg::Screen::ClearScreenStr;
  ::std::string EmptyStr;
  EmptyStr.resize(11 * 80, ' ');
  Ref << EmptyStr;
  EmptyStr.resize(34, ' ');
  Ref << EmptyStr << "Hello World!" << EmptyStr;
  EmptyStr.resize(12 * 80, ' ');
  Ref << EmptyStr;

  ::cxxg::Screen Screen(80, 24, SS);
  Screen[11][34] = "Hello World!";
  Screen.update();

  ::std::cout << SS.str();

  // unequal since we want to return true for failure
  return SS.str() != Ref.str();
}
