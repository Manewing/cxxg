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

  void warn(::std::string Warnings);

protected:
  Screen Scr;
  bool GameRunning;
  ::std::vector<::std::string> Warnings;
};

}; // namespace cxxg

#endif // #ifndef CXXG_GAME_H
