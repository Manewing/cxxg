#include "Common.h"

namespace {

TEST(cxxg, Colors) {
  // create row
  ::cxxg::Row Row(10);
  ::std::stringstream SS;

  // for generating references
  ::std::stringstream Ref;
  ::std::vector<::cxxg::types::Color> ColorRef;
  ColorRef.resize(10, ::cxxg::types::Color::NONE);

  // check that init worked correctly
  EXPECT_EQ(Row.getColorInfo(), ColorRef) << "RowColorInit";

  // check adding new color works correctly
  Row[0] << ::cxxg::types::Color::BLUE << "test";
  ::std::fill(ColorRef.begin(), ColorRef.begin() + 4,
              ::cxxg::types::Color::BLUE);
  EXPECT_EQ(Row.getColorInfo(), ColorRef) << "RowBasicColor";

  // check adding new color with positive offset works correctly
  Row[8] << ::cxxg::types::Color::GREEN << "test";
  ::std::fill(ColorRef.begin() + 8, ColorRef.end(),
              ::cxxg::types::Color::GREEN);
  EXPECT_EQ(Row.getColorInfo(), ColorRef) << "RowOffsetPositive";

  // check adding new color with negative offset works correctly
  Row[-2] << ::cxxg::types::Color::RED << "test";
  ::std::fill(ColorRef.begin(), ColorRef.begin() + 2,
              ::cxxg::types::Color::RED);
  EXPECT_EQ(Row.getColorInfo(), ColorRef) << "RowOffsetNegative";

  // check clear
  Row.clear();
  ::std::fill(ColorRef.begin(), ColorRef.begin(), ::cxxg::types::Color::NONE);

  // check output
  Row[0] << ::cxxg::types::Color::RED << "test";
  Row[1] = ::cxxg::types::Color::GREEN;
  Row[2] = ::cxxg::types::Color::BLUE;
  Row.dump(SS);
  Ref << "\033[31mt\033[32me\033[34ms\033[31mt\033[0m      \033[0m";
  EXPECT_EQ(SS.str(), Ref.str()) << "RowColoredOutput";
}

} // namespace
