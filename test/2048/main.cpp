#include "Common.h"
#include "Game2048.h"

::std::ostream &operator<<(::std::ostream &Out, ::cxxg::Color Cl) {
  Out << Cl.Value;
  return Out;
}

::std::ostream &
operator<<(::std::ostream &Out,
           ::std::vector<::std::vector<unsigned>> const &Board) {
  Out << ::std::endl;

  for (auto const &Line : Board) {
    Out << "   " << Line << ::std::endl;
  }

  Out << ::std::endl;
  return Out;
}

void checkLineMoveRight(::std::vector<unsigned> Line,
                        ::std::vector<unsigned> LineRef, unsigned ScoreRef) {
  unsigned Score = Game2048::moveLineRight(Line);
  EXPECT_EQ(Line, LineRef);
  EXPECT_EQ(Score, ScoreRef);
}

void checkRotation(::std::vector<::std::vector<unsigned>> Board,
                   ::std::vector<::std::vector<unsigned>> BoardRef) {
  Game2048::rotateBoard(Board);
  EXPECT_EQ(Board, BoardRef);
}

int main() {

  // check correctness of get power function
  EXPECT_EQ(Game2048::getPowerOfTwo(2), 1);
  EXPECT_EQ(Game2048::getPowerOfTwo(4), 2);
  EXPECT_EQ(Game2048::getPowerOfTwo(8), 3);
  EXPECT_EQ(Game2048::getPowerOfTwo(32), 5);
  EXPECT_EQ(Game2048::getPowerOfTwo(4096), 12);

  // check correct switching between colors
  EXPECT_EQ(Game2048::getColor(2), ::cxxg::Color::BLUE);
  EXPECT_EQ(Game2048::getColor(32), ::cxxg::Color::BLUE);
  EXPECT_EQ(Game2048::getColor(64), ::cxxg::Color::GREEN);
  EXPECT_EQ(Game2048::getColor(8), ::cxxg::Color::YELLOW);
  EXPECT_EQ(Game2048::getColor(16), ::cxxg::Color::RED);

  // check moving of line
  checkLineMoveRight({0, 2, 0, 0}, {0, 0, 0, 2}, 0);
  checkLineMoveRight({2, 0, 0, 0}, {0, 0, 0, 2}, 0);
  checkLineMoveRight({0, 0, 0, 4}, {0, 0, 0, 4}, 0);
  checkLineMoveRight({0, 0, 2, 4}, {0, 0, 2, 4}, 0);

  // check moving of line with merging
  checkLineMoveRight({0, 2, 0, 2}, {0, 0, 0, 4}, 4);
  checkLineMoveRight({2, 2, 2, 2}, {0, 0, 4, 4}, 8);
  checkLineMoveRight({2, 2, 4, 8}, {0, 4, 4, 8}, 4);
  checkLineMoveRight({4, 2, 2, 8}, {0, 0, 8, 8}, 12);
  checkLineMoveRight({8, 4, 2, 2}, {0, 0, 0, 16}, 28);

  // check move full and empty
  checkLineMoveRight({2, 4, 8, 16}, {2, 4, 8, 16}, 0);
  checkLineMoveRight({0, 0, 0, 0}, {0, 0, 0, 0}, 0);

  // check if rotation works correctly
  checkRotation({{0, 1, 2, 3}, {0, 1, 2, 3}, {0, 1, 2, 3}, {0, 1, 2, 3}},
                {{3, 3, 3, 3}, {2, 2, 2, 2}, {1, 1, 1, 1}, {0, 0, 0, 0}});

  // check if move works correctly
  {
    ::std::vector<::std::vector<unsigned>> Board = {
        {0, 0, 2, 0}, {0, 0, 2, 0}, {0, 0, 2, 0}, {0, 0, 2, 0}};
    ::std::vector<::std::vector<unsigned>> BoardRef = {
        {0, 0, 0, 2}, {0, 0, 0, 2}, {0, 0, 0, 2}, {0, 0, 0, 2}};
    Game2048::moveRight(Board);
    EXPECT_EQ_MSG(Board, BoardRef, "moveRight");
  }
  {
    ::std::vector<::std::vector<unsigned>> Board = {
        {0, 0, 2, 0}, {0, 0, 2, 0}, {0, 0, 2, 0}, {0, 0, 2, 0}};
    ::std::vector<::std::vector<unsigned>> BoardRef = {
        {0, 0, 0, 2}, {0, 0, 0, 2}, {0, 0, 0, 2}, {0, 0, 0, 2}};
    Game2048::rotateBoard(Board);
    Game2048::rotateBoard(BoardRef);
    Game2048::rotateBoard(BoardRef);
    Game2048::rotateBoard(BoardRef);
    Game2048::moveDown(Board);
    EXPECT_EQ_MSG(Board, BoardRef, "moveDown");
  }
  {
    ::std::vector<::std::vector<unsigned>> Board = {
        {0, 0, 2, 0}, {0, 0, 2, 0}, {0, 0, 2, 0}, {0, 0, 2, 0}};
    ::std::vector<::std::vector<unsigned>> BoardRef = {
        {0, 0, 0, 2}, {0, 0, 0, 2}, {0, 0, 0, 2}, {0, 0, 0, 2}};
    Game2048::rotateBoard(Board);
    Game2048::rotateBoard(Board);
    Game2048::rotateBoard(BoardRef);
    Game2048::rotateBoard(BoardRef);
    Game2048::moveLeft(Board);
    EXPECT_EQ_MSG(Board, BoardRef, "moveLeft");
  }
  {
    ::std::vector<::std::vector<unsigned>> Board = {
        {0, 0, 2, 0}, {0, 0, 2, 0}, {0, 0, 2, 0}, {0, 0, 2, 0}};
    ::std::vector<::std::vector<unsigned>> BoardRef = {
        {0, 0, 0, 2}, {0, 0, 0, 2}, {0, 0, 0, 2}, {0, 0, 0, 2}};
    Game2048::rotateBoard(Board);
    Game2048::rotateBoard(Board);
    Game2048::rotateBoard(Board);
    Game2048::rotateBoard(BoardRef);
    Game2048::moveUp(Board);
    EXPECT_EQ_MSG(Board, BoardRef, "moveUp");
  }

  RETURN_SUCCESS;
}
