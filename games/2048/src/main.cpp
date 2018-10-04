#include "Game2048.h"

#include <cxxg/Utils.h>

int main() {

  // get path to highscore file
  auto HighScoreFile = ::cxxg::utils::getHomeDir();
  HighScoreFile += "/.cxxg.2048.h.s";

  // create gane instance
  Game2048 Game(HighScoreFile);

  // register handler for Ctrl+C
  ::cxxg::utils::registerSigintHandler([&Game]() {
    Game.handleGameOver(false);
    Game.handleExit();
    exit(0);
  });

  // run game
  Game.initialize();
  Game.run();

  return 0;
}
