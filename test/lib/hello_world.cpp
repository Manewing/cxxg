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
  Screen[11][34] << "Hello World!";
  Screen.update();
  ::std::cout << SS.str() << ::std::endl;

  // create reference
  Ref << ::cxxg::Screen::ClearScreenStr << ::cxxg::Screen::HideCursorStr;
  EmptyStr.resize(80, ' ');
  for (int l = 0; l < 11; l++) {
    Ref << EmptyStr << "\033[0m";
  }
  EmptyStr.resize(34, ' ');
  Ref << EmptyStr << "Hello World!" << EmptyStr << "\033[0m";
  EmptyStr.resize(80, ' ');
  for (int l = 0; l < 12; l++) {
    Ref << EmptyStr << "\033[0m";
  }
  Ref << ::cxxg::Screen::ShowCursorStr;

  // check that reference matches
  EXPECT_EQ_MSG(SS.str(), Ref.str(), "Hello World");
  SS.str("");
  Ref.str("");

  // check that clear screen works correctly
  Screen.clear();
  Screen.update();
  Ref << ::cxxg::Screen::ClearScreenStr << ::cxxg::Screen::HideCursorStr;
  for (int l = 0; l < 24; l++) {
    Ref << EmptyStr << "\033[0m";
  }
  Ref << ::cxxg::Screen::ShowCursorStr;
  EXPECT_EQ_MSG(SS.str(), Ref.str(), "Clear Screen");
  SS.clear();
  Ref.clear();

  RETURN_SUCCESS;
}
