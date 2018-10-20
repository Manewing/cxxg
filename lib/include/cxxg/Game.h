#ifndef CXXG_GAME_H
#define CXXG_GAME_H

#include <cxxg/Screen.h>

#include <memory>
#include <string>
#include <vector>

namespace cxxg {

class Game {
public:
  Game();
  virtual ~Game();

  /// Initializes environment
  virtual void initialize(bool BufferedInput);

  void run();

  virtual void handleInput(int Char) = 0;

  virtual void draw();

  virtual void handleGameOver() = 0;

  RowAccessor warn();

  void checkSize(ScreenSize GameSize) const;
  ScreenSize getOffset(ScreenSize GameSize) const;

protected:
  Screen Scr;
  bool GameRunning;
  ::std::vector<Row> Warnings;
};

}; // namespace cxxg

#endif // #ifndef CXXG_GAME_H