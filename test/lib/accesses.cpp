#include "Common.h"
#include <cxxg/Screen.h>

namespace {

TEST(cxxg, Accesses) {
  // buffer in string stream to check results later
  ::std::stringstream SS;
  ::cxxg::Screen Screen(::cxxg::types::Size{10, 1}, SS, false);
  ::cxxg::Row Row(10);

  // string stream for generating reference
  ::std::string EmptyStr;
  ::std::stringstream Ref;

  // check zero offset access to row
  Row[0] << "test";
  EmptyStr.resize(6, ' ');
  Ref << "test" << EmptyStr;
  EXPECT_EQ(Row.getBuffer(), Ref.str()) << "RowAccessZeroOffset";

  // clear everything
  Screen.clear();
  Row.clear();
  SS.str("");
  Ref.str("");

  // check positive offset access to row
  Row[2] << "test";
  EmptyStr.resize(4, ' ');
  Ref << "  "
      << "test" << EmptyStr;
  EXPECT_EQ(Row.getBuffer(), Ref.str()) << "RowAccessPositiveOffset";

  // clear everything
  Screen.clear();
  Row.clear();
  SS.str("");
  Ref.str("");

  // check access to row with a negative offset
  Row[-2] << "test";
  EmptyStr.resize(8, ' ');
  Ref << "st" << EmptyStr;
  EXPECT_EQ(Row.getBuffer(), Ref.str()) << "RowAcessNegativeOffset";

  // clear everything
  Screen.clear();
  Row.clear();
  SS.str("");
  Ref.str("");

  // check access to row, writing "over the edge"
  Row[8] << "test";
  EmptyStr.resize(8, ' ');
  Ref << EmptyStr << "te";
  EXPECT_EQ(Row.getBuffer(), Ref.str()) << "AccessRowOffsetEnd";

  // clear everything
  Screen.clear();
  Row.clear();
  SS.str("");
  Ref.str("");

  // check access to row, writing "at the edge"
  Row[10] << "test";
  EmptyStr.resize(10, ' ');
  Ref << EmptyStr;
  EXPECT_EQ(Row.getBuffer(), Ref.str()) << "AccessRowOffsetEnd";

  // clear everything
  Screen.clear();
  Row.clear();
  SS.str("");
  Ref.str("");

  // check that out of range accesses to rows are ignored
  Screen[-1][0] << "test";
  Screen[10][0] << "test";
  Screen.update();
  EmptyStr.resize(10, ' ');
  Ref << ::cxxg::Screen::ClearScreenStr << ::cxxg::Screen::HideCursorStr
      << EmptyStr << "\033[0m" << ::cxxg::Screen::ShowCursorStr;
  EXPECT_EQ(SS.str(), Ref.str()) << "RowOutOfRange";
}

} // namespace
