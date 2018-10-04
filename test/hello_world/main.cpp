#include <cxxg/Screen.h>

#include <sstream>

#define EXPECT_TRUE(stmt)                                                      \
  if (!(stmt)) {                                                               \
    ::std::cerr << "ERROR: expected statement '" << #stmt << "' to be true"    \
                << ::std::endl;                                                \
    Success = false;                                                           \
  }

int main() {
  bool Success = true;

  // buffer in string stream to check results later
  ::std::stringstream SS;
  ::cxxg::Screen Screen(80, 24, SS);

  // string stream for generating reference
  ::std::string EmptyStr;
  ::std::stringstream Ref;

  // test simple writing of hello world
  Screen[11][34] = "Hello World!";
  Screen.update();
  ::std::cout << SS.str() << ::std::endl;

  // create reference
  Ref << ::cxxg::Screen::ClearScreenStr;
  EmptyStr.resize(80, ' ');
  for (int l = 0; l < 11; l++) {
    Ref << EmptyStr << "\033[0m";
  }
  EmptyStr.resize(34, ' ');
  Ref << EmptyStr << "Hello World!"
      << "\033[0m" << EmptyStr << "\033[0m";
  EmptyStr.resize(80, ' ');
  for (int l = 0; l < 12; l++) {
    Ref << EmptyStr << "\033[0m";
  }

  // check that reference matches
  EXPECT_TRUE(SS.str() == Ref.str());
  SS.str("");
  Ref.str("");

  // check that last update cleared the screen, another update
  // will write last state into SS
  Screen.update();
  Ref << ::cxxg::Screen::ClearScreenStr;
  for (int l = 0; l < 24; l++) {
    Ref << EmptyStr << "\033[0m";
  }
  EXPECT_TRUE(SS.str() == Ref.str());
  SS.clear();
  Ref.clear();

  if (Success) {
    return 0;
  }
  return 1;
}
