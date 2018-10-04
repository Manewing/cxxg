#ifndef CXXG_GAME_H
#define CXXG_GAME_H

#include <cxxg/Screen.h>

#include <memory>

namespace cxxg {

class Game {
public:
  Game(size_t SizeX, size_t SizeY);
  virtual ~Game();

  /// Initializes environment
  virtual void initialize(bool BufferedInput);

  void run();

  virtual void handleInput(int Char) = 0;

  virtual void draw() = 0;

  virtual void handleGameOver(bool Victory) = 0;

protected:
  Screen Scr;
  bool GameRunning;
};

}; // namespace cxxg

#endif // #ifndef CXXG_GAME_H
