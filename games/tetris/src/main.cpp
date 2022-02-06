#include <Tetris.h>
#include <cxxg/Utils.h>
#include <stdexcept>

int main() {
  cxxg::Screen Scr(cxxg::Screen::getTerminalSize());
  cxxg::utils::registerSigintHandler([]() { exit(0); });

  Tetris Game(Scr);
  try {
    Game.initialize(false, 50000);
    Game.run(false);
  } catch (std::exception const &E) {
    std::cerr << "ERROR: " << E.what() << std::endl;
    return 1;
  }

  return 0;
}