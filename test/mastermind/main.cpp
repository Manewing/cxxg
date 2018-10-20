#include "Common.h"
#include "MasterMind.h"

int main() {

  // check black and white pin calculation
  ::std::vector<char> Code = {'c', 'd', 'a', 'e'};
  ::std::vector<char> Guess = {' ', ' ', ' ', ' '};

  EXPECT_EQ_MSG(::MasterMind::getWhites(Code, Guess), 0, "WhitesEmptyGuess");
  EXPECT_EQ_MSG(::MasterMind::getBlacks(Code, Guess), 0, "BlacksEmptyGuess");
  EXPECT_EQ_MSG(::MasterMind::isValidGuess(Guess), false, "InvalidGuess");

  Guess = {'g', 'c', 'f', 'b'};
  EXPECT_EQ_MSG(::MasterMind::getWhites(Code, Guess), 1, "WhitesOneGuess");
  EXPECT_EQ_MSG(::MasterMind::getBlacks(Code, Guess), 0, "WhitesOneGuess");
  EXPECT_EQ_MSG(::MasterMind::isValidGuess(Guess), true, "ValidGuess");

  Guess = {'c', 'g', 'f', 'b'};
  EXPECT_EQ_MSG(::MasterMind::getWhites(Code, Guess), 0, "BlacksOneGuess");
  EXPECT_EQ_MSG(::MasterMind::getBlacks(Code, Guess), 1, "BlacksOneGuess");

  Guess = {'g', 'c', 'e', 'f'};
  EXPECT_EQ_MSG(::MasterMind::getWhites(Code, Guess), 2, "WhitesTwoGuess");
  EXPECT_EQ_MSG(::MasterMind::getBlacks(Code, Guess), 0, "WhitesTwoGuess");

  Guess = {'c', 'd', 'e', 'a'};
  EXPECT_EQ_MSG(::MasterMind::getWhites(Code, Guess), 2, "BWTwoGuess");
  EXPECT_EQ_MSG(::MasterMind::getBlacks(Code, Guess), 2, "BWTwoGuess");

  Guess = {'c', 'd', 'a', 'e'};
  EXPECT_EQ_MSG(::MasterMind::getWhites(Code, Guess), 0, "CorrectGuess");
  EXPECT_EQ_MSG(::MasterMind::getBlacks(Code, Guess), 4, "CorrectGuess");

  RETURN_SUCCESS;
}
