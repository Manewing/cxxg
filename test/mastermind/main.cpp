#include "Common.h"
#include "MasterMind.h"

namespace {

// check black and white pin calculation
TEST(MasterMind, Guess) {
  ::std::vector<char> Code = {'c', 'd', 'a', 'e'};
  ::std::vector<char> Guess = {' ', ' ', ' ', ' '};

  EXPECT_EQ(::MasterMind::getWhites(Code, Guess), 0) << "WhitesEmptyGuess";
  EXPECT_EQ(::MasterMind::getBlacks(Code, Guess), 0) << "BlacksEmptyGuess";
  EXPECT_EQ(::MasterMind::isValidGuess(Guess), false) << "InvalidGuess";

  Guess = {'g', 'c', 'f', 'b'};
  EXPECT_EQ(::MasterMind::getWhites(Code, Guess), 1) << "WhitesOneGuess";
  EXPECT_EQ(::MasterMind::getBlacks(Code, Guess), 0) << "WhitesOneGuess";
  EXPECT_EQ(::MasterMind::isValidGuess(Guess), true) << "ValidGuess";

  Guess = {'c', 'g', 'f', 'b'};
  EXPECT_EQ(::MasterMind::getWhites(Code, Guess), 0) << "BlacksOneGuess";
  EXPECT_EQ(::MasterMind::getBlacks(Code, Guess), 1) << "BlacksOneGuess";

  Guess = {'g', 'c', 'e', 'f'};
  EXPECT_EQ(::MasterMind::getWhites(Code, Guess), 2) << "WhitesTwoGuess";
  EXPECT_EQ(::MasterMind::getBlacks(Code, Guess), 0) << "WhitesTwoGuess";

  Guess = {'c', 'd', 'e', 'a'};
  EXPECT_EQ(::MasterMind::getWhites(Code, Guess), 2) << "BWTwoGuess";
  EXPECT_EQ(::MasterMind::getBlacks(Code, Guess), 2) << "BWTwoGuess";

  Guess = {'c', 'd', 'a', 'e'};
  EXPECT_EQ(::MasterMind::getWhites(Code, Guess), 0) << "CorrectGuess";
  EXPECT_EQ(::MasterMind::getBlacks(Code, Guess), 4) << "CorrectGuess";

  Guess = {};
  EXPECT_THROW(::MasterMind::getWhites(Code, Guess), ::std::runtime_error);
  EXPECT_THROW(::MasterMind::getBlacks(Code, Guess), ::std::runtime_error);
}

// check random code generation
TEST(MasterMind, RandomCode) {
  cxxg::Screen Scr(cxxg::types::Size{100, 100});
  ::MasterMind MM(Scr);

  EXPECT_EQ(::MasterMind::isValidGuess(MM.getRandomCode(1, 1)), true);
  EXPECT_EQ(::MasterMind::isValidGuess(MM.getRandomCode(1, 10)), true);
  EXPECT_EQ(::MasterMind::isValidGuess(MM.getRandomCode(10, 10)), true);
  EXPECT_EQ(::MasterMind::isValidGuess(MM.getRandomCode(5, 10)), true);
  EXPECT_THROW(MM.getRandomCode(1, 0), ::std::runtime_error);
  EXPECT_THROW(MM.getRandomCode(10, 1000), ::std::runtime_error);
}

} // namespace
