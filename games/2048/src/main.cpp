#include "Game2048.h"

#include <cxxg/Utils.h>

int main() {
  Game2048 Game;

  ::cxxg::utils::registerSigintHandler([&Game]() {
    Game.handleGameOver(false);
    Game.draw();
    exit(0);
  });

  Game.initialize();
  Game.run();

  return 0;
}
