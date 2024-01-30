#include <gtest/gtest.h>
#include <rogue/UI/WordWrap.h>

namespace {

TEST(WordWrapTest, EmptyString) {
  rogue::ui::WordWrap WW("", 10);
  EXPECT_EQ(WW.getNumLines(), 1);
  EXPECT_EQ(WW.getLineWidth(), 10);
  EXPECT_EQ(WW.getLine(0), " ");
  EXPECT_THROW(WW.getLine(1), std::out_of_range);
}

TEST(WordWrapTest, SingleWord) {
  rogue::ui::WordWrap WW("Hello", 10);
  EXPECT_EQ(WW.getNumLines(), 1);
  EXPECT_EQ(WW.getLineWidth(), 10);
  EXPECT_EQ(WW.getLine(0), "Hello ");
}

TEST(WordWrapTest, SingleReallyLongWord) {
  rogue::ui::WordWrap WW("Hellooooooooooooooooooooooooooooo", 10);
  EXPECT_EQ(WW.getNumLines(), 2);
  EXPECT_EQ(WW.getLine(0), "");
  EXPECT_EQ(WW.getLine(1), "Hellooooooooooooooooooooooooooooo ");
}

TEST(WordWrapTest, MultipleWords) {
  rogue::ui::WordWrap WW("Hello to this World", 10);
  EXPECT_EQ(WW.getNumLines(), 3);
  EXPECT_EQ(WW.getLine(0), "Hello to ");
  EXPECT_EQ(WW.getLine(1), "this ");
  EXPECT_EQ(WW.getLine(2), "World ");
}

TEST(WordWrapTest, MultipleWordsOneReallyLong) {
  rogue::ui::WordWrap WW("Hello toooooooooooooooooooooooooooooooo this World",
                         10);
  EXPECT_EQ(WW.getNumLines(), 4);
  EXPECT_EQ(WW.getLine(0), "Hello ");
  EXPECT_EQ(WW.getLine(1), "toooooooooooooooooooooooooooooooo ");
  EXPECT_EQ(WW.getLine(2), "this ");
  EXPECT_EQ(WW.getLine(3), "World ");
}

TEST(WordWrapTest, MultipleWordsWithNewLine) {
  rogue::ui::WordWrap WW("Hello\nto this World", 10);
  EXPECT_EQ(WW.getNumLines(), 3);
  EXPECT_EQ(WW.getLine(0), "Hello ");
  EXPECT_EQ(WW.getLine(1), "to this ");
  EXPECT_EQ(WW.getLine(2), "World ");
}

TEST(WordWrapTest, NewLineExactlyAtEndOfLine) {
  rogue::ui::WordWrap WW("123456789\n", 10);
  EXPECT_EQ(WW.getNumLines(), 2);
  EXPECT_EQ(WW.getLine(0), "123456789 ");
  EXPECT_EQ(WW.getLine(1), " ");
}

TEST(WordWrapTest, NewLineAfterEndOfLine) {
  rogue::ui::WordWrap WW("0123456789\n", 10);
  // FIXME we should get only two lines here
  EXPECT_EQ(WW.getNumLines(), 3);
  EXPECT_EQ(WW.getLine(0), "");
  EXPECT_EQ(WW.getLine(1), "0123456789 ");
  EXPECT_EQ(WW.getLine(2), " ");
}

TEST(WordWrapTest, NewLineBeforeEndOfLine) {
  rogue::ui::WordWrap WW("\n0123456789", 10);
  EXPECT_EQ(WW.getNumLines(), 3);
  EXPECT_EQ(WW.getLine(0), " ");
  EXPECT_EQ(WW.getLine(1), "");
  EXPECT_EQ(WW.getLine(2), "0123456789 ");
}

} // namespace