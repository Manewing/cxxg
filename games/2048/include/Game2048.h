#include <cxxg/Game.h>

class Game2048 : public ::cxxg::Game {
public:
  typedef enum { Running, GameOver, Victory } GameState;

public:
  Game2048();

  void initialize(bool BufferedInput = true) final;

  void handleInput(int Char) final;

  void draw() final;

  void handleGameOver(bool Victory) final;

  void addNewElement();

  static int getPowerOfTwo(unsigned Element);
  static ::cxxg::Color getColor(unsigned Element);

  static bool hasFreeSpace(::std::vector<::std::vector<unsigned>> &Board);

  static unsigned moveRight(::std::vector<::std::vector<unsigned>> &Board);
  static unsigned moveLeft(::std::vector<::std::vector<unsigned>> &Board);
  static unsigned moveUp(::std::vector<::std::vector<unsigned>> &Board);
  static unsigned moveDown(::std::vector<::std::vector<unsigned>> &Board);

  static void rotateBoard(::std::vector<::std::vector<unsigned>> &Board);
  static unsigned moveLineRight(::std::vector<unsigned> &Line);

private:
  unsigned Score;
  unsigned Highscore;
  ::std::vector<::std::vector<unsigned>> Board;
  GameState State;
};
