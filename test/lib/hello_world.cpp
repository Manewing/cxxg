#include "Common.h"
#include <cxxg/Screen.h>

int main() {
  // buffer in string stream to check results later
  ::std::stringstream SS;
  ::cxxg::Screen Screen(::cxxg::ScreenSize{80, 24}, SS);

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
  EXPECT_EQ(SS.str(), Ref.str());
  SS.str("");
  Ref.str("");

  // check that last update cleared the screen, another update
  // will write last state into SS
  Screen.update();
  Ref << ::cxxg::Screen::ClearScreenStr;
  for (int l = 0; l < 24; l++) {
    Ref << EmptyStr << "\033[0m";
  }
  EXPECT_EQ(SS.str(), Ref.str());
  SS.clear();
  Ref.clear();

  RETURN_SUCCESS;
}
