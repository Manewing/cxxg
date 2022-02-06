#include <Tetris.h>
#include <cxxg/Utils.h>
#include <stdexcept>

int main() {
  cxxg::Screen Scr(cxxg::Screen::getTerminalSize());

  Tetris Game(Scr);
  Game.initialize(false, 50000);
  Game.run(false);
  return 0;

  cxxg::utils::registerSigintHandler([]() { exit(0); });

  try {
  } catch (std::exception const &E) {
    std::cerr << "ERROR: " << E.what() << std::endl;
    return 1;
  }

  return 0;
}