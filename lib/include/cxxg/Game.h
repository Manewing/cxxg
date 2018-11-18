#ifndef CXXG_GAME_H
#define CXXG_GAME_H

#include <cxxg/Screen.h>

#include <memory>
#include <random>
#include <string>
#include <vector>

namespace cxxg {

class Game {
public:
  Game();

  /// Initializes environment
  virtual void initialize(bool BufferedInput);

  void run();

  virtual void handleInput(int Char) = 0;

  virtual void draw();

  virtual void handleGameOver() = 0;

  virtual void handleExit();

  RowAccessor warn();

  void checkSize(ScreenSize GameSize) const;

  ScreenSize getOffset(ScreenSize GameSize) const;

  void setRandomSeed(size_t Seed);

protected:
  /// Screen on which the game will be displayed
  Screen Scr;

  /// Flag for game loop determining whether the game is running
  bool GameRunning;

  /// Buffer for warnings to display
  ::std::vector<Row> Warnings;

  /// Random engine for generating random numbers in games
  ::std::default_random_engine RndEngine;
};

}; // namespace cxxg

#endif // #ifndef CXXG_GAME_H
