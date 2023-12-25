#include "Game2048.h"

#include <cxxg/Utils.h>
#include <stdexcept>

int main() {

  // create screen for game
  ::cxxg::Screen Scr(cxxg::Screen::getTerminalSize());

  // get path to highscore file
  auto HighScoreFile = ::cxxg::utils::getHomeDir();
  HighScoreFile /= ".cxxg.2048.h.s";

  // create gane instance
  Game2048 Game(Scr, HighScoreFile.string());

  // register handler for Ctrl+C
  ::cxxg::utils::registerSigintHandler([&Game]() {
    Game.handleGameOver();
    Game.handleExit();
    exit(0);
  });

  // run game
  try {
    Game.initialize();
    Game.run();
  } catch (::std::exception const &E) {
    ::std::cerr << "ERROR: " << E.what() << ::std::endl;
    return 1;
  }

  return 0;
}
