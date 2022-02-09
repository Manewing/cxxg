#include <cxxg/Game.h>

class Game2048 : public ::cxxg::Game {
public:
  typedef enum { Running, GameOver, NewGame } GameState;

public:
  Game2048(::cxxg::Screen &Scr, ::std::string const &HighScoreFile);
  virtual ~Game2048();

  void initialize(bool BufferedInput = false, unsigned TickDelayUs = 0) final;

  bool handleInput(int Char) final;

  void handleDraw() final;
  void drawLogoAnimation();

  void handleGameOver();

  void handleExit() final;

  void addNewElement();

  static int getPowerOfTwo(unsigned Element);
  static ::cxxg::types::TermColor getColor(unsigned Idx);
  static ::cxxg::types::TermColor getElemColor(unsigned Element);

  static bool hasFreeSpace(::std::vector<::std::vector<unsigned>> &Board);

  static unsigned moveRight(::std::vector<::std::vector<unsigned>> &Board);
  static unsigned moveLeft(::std::vector<::std::vector<unsigned>> &Board);
  static unsigned moveUp(::std::vector<::std::vector<unsigned>> &Board);
  static unsigned moveDown(::std::vector<::std::vector<unsigned>> &Board);

  static void rotateBoard(::std::vector<::std::vector<unsigned>> &Board);
  static unsigned moveLineRight(::std::vector<unsigned> &Line);

private:
  unsigned Score;
  unsigned HighScore;
  ::std::vector<::std::vector<unsigned>> Board;
  GameState State;
  ::std::string HighScoreFile;
};
