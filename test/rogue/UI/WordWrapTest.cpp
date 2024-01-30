#include <gtest/gtest.h>
#include <rogue/UI/WordWrap.h>
#include <string>
#include <vector>

namespace {

using LinesTy = std::vector<std::string>;
LinesTy getLines(const rogue::ui::WordWrap &WW) {
  LinesTy Lines;
  for (std::size_t i = 0; i < WW.getNumLines(); ++i) {
    EXPECT_LE(WW.getLine(i).size(), WW.getLineWidth());
    Lines.push_back(std::string(WW.getLine(i)));
  }
  return Lines;
}

TEST(WordWrapTest, EmptyString) {
  rogue::ui::WordWrap WW("", 10);
  EXPECT_EQ(WW.getNumLines(), 1);
  EXPECT_EQ(WW.getLineWidth(), 10);
  EXPECT_EQ(WW.getLine(0), " ");
  EXPECT_THROW(WW.getLine(1), std::out_of_range);
}

TEST(WordWrapTest, SingleWord) {
  rogue::ui::WordWrap WW("Hello", 10);
  EXPECT_EQ(WW.getLineWidth(), 10);
  EXPECT_EQ(getLines(WW), LinesTy{"Hello"});
}

TEST(WordWrapTest, SingleWordExactlyMatchingLineWidth) {
  std::string Text = "123456789";
  rogue::ui::WordWrap WW(Text, Text.size());
  LinesTy Ref = {Text};
  EXPECT_EQ(getLines(WW), Ref);
}

TEST(WordWrapTest, SingleReallyLongWord) {
  rogue::ui::WordWrap WW("Hellooooooooooooooooooooooooooooo", 10);
  LinesTy Ref = {"Helloooooo", "oooooooooo", "oooooooooo", "ooo"};
  EXPECT_EQ(getLines(WW), Ref);
}

TEST(WordWrapTest, SingleReallyLongWordWithTrailingSpace) {
  rogue::ui::WordWrap WW("Hellooooooooooooooooooooooooooooo ", 10);
  LinesTy Ref = {"Helloooooo", "oooooooooo", "oooooooooo", "ooo "};
  EXPECT_EQ(getLines(WW), Ref);
}

TEST(WordWrapTest, SingleReallyLongWordWithLeadingSpace) {
  rogue::ui::WordWrap WW(" Hellooooooooooooooooooooooooooooo", 10);
  LinesTy Ref = {" Hellooooo", "oooooooooo", "oooooooooo", "oooo"};
  EXPECT_EQ(getLines(WW), Ref);
}

TEST(WordWrapTest, MultipleWords) {
  rogue::ui::WordWrap WW("Hello to this World", 10);
  LinesTy Ref = {"Hello to ", "this World"};
  EXPECT_EQ(getLines(WW), Ref);
}

TEST(WordWrapTest, MultipleWordsOneReallyLong) {
  rogue::ui::WordWrap WW(
      "Hello tooooooooooooooooooooooooooooooooooo this World", 10);
  LinesTy Ref = {"Hello tooo", "oooooooooo", "oooooooooo",
                 "oooooooooo", "oo this ",   "World"};
  EXPECT_EQ(getLines(WW), Ref);
}

TEST(WordWrapTest, MultipleWordsOneReallyLongNewLine) {
  rogue::ui::WordWrap WW(
      "Hello tooooooooooooooooooooooooooooooooooo\nthis World", 10);
  LinesTy Ref = {"Hello tooo", "oooooooooo", "oooooooooo",
                 "oooooooooo", "oo",         "this World"};
  EXPECT_EQ(getLines(WW), Ref);
}

TEST(WordWrapTest, MultipleWordsWithNewLine) {
  rogue::ui::WordWrap WW("Hello\nto this World", 10);
  LinesTy Ref = {"Hello", "to this ", "World"};
  EXPECT_EQ(getLines(WW), Ref);
}

TEST(WordWrapTest, NewLineExactlyAtEndOfLine) {
  rogue::ui::WordWrap WW("123456789\n", 10);
  LinesTy Ref = {"123456789", " "};
  EXPECT_EQ(getLines(WW), Ref);
}

TEST(WordWrapTest, NewLineAfterEndOfLine) {
  rogue::ui::WordWrap WW("0123456789\n", 10);
  LinesTy Ref = {"0123456789", " "};
  EXPECT_EQ(getLines(WW), Ref);
}

TEST(WordWrapTest, NewLineBeforeEndOfLine) {
  rogue::ui::WordWrap WW("\n0123456789", 10);
  LinesTy Ref = {"", "0123456789"};
  EXPECT_EQ(getLines(WW), Ref);
}

} // namespace