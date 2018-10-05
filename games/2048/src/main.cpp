#include "Game2048.h"

#include <cxxg/Utils.h>
#include <stdexcept>

int main() {

  // get path to highscore file
  auto HighScoreFile = ::cxxg::utils::getHomeDir();
  HighScoreFile += "/.cxxg.2048.h.s";

  // create gane instance
  Game2048 Game(HighScoreFile);

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
    exit(1);
  }

  return 0;
}
