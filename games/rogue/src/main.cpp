#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <rogue/Game.h>
#include <rogue/GameConfig.h>
#include <stdexcept>

int main(int Argc, char *Argv[]) {
  if (Argc != 2) {
    std::cout << "Usage: " << Argv[0] << " <game_config.json>" << std::endl;
    return 0;
  }
  auto Cfg = rogue::GameConfig::load(Argv[1]);

  cxxg::Screen Scr(cxxg::Screen::getTerminalSize());
  cxxg::utils::registerSigintHandler([]() { exit(0); });

  rogue::Game GameInstance(Scr, Cfg);
  try {
    GameInstance.initialize();
    GameInstance.run();
  } catch (std::exception const &E) {
    std::cerr << "ERROR: " << E.what() << std::endl;
    return 1;
  }

  return 0;
}
