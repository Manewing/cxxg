#include <cxxg/Utils.h>
#include <cxxg/Screen.h>
#include <stdexcept>
#include <rogue/Game.h>

int main() {
  cxxg::Screen Scr(cxxg::Screen::getTerminalSize());
  cxxg::utils::registerSigintHandler([]() { exit(0); });

  rogue::Game GameInstance(Scr);
  try {
    GameInstance.initialize();
    GameInstance.run();
  } catch (std::exception const &E) {
    std::cerr << "ERROR: " << E.what() << std::endl;
    return 1;
  }

  return 0;
}
