#include <MasterMind.h>
#include <cxxg/Utils.h>
#include <stdexcept>

int main() {

  // Create the screen
  cxxg::Screen Scr(cxxg::Screen::getTerminalSize());

  // create game instance
  MasterMind Game(Scr);

  // register handler for Ctrl+C
  ::cxxg::utils::registerSigintHandler([&Game]() {
    // call handle exit, since due to 'exit' destructors are not invoked
    Game.handleGameOver();
    Game.handleExit();
    exit(0);
  });

  try {
    Game.initialize();
    Game.run();
  } catch (::std::exception const &E) {
    ::std::cerr << "ERROR: " << E.what() << ::std::endl;
    return 1;
  }

  return 0;
}
