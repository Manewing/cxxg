#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <rogue/Game.h>
#include <rogue/GameConfig.h>
#include <stdexcept>
#include <string_view>

#ifndef WIN32

#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

void handler(int) {
  void *StackFrames[40];
  size_t NumFrames = 0;
  std::cerr << "SEGFAULT" << std::endl;
  NumFrames = backtrace(StackFrames, 40);
  backtrace_symbols_fd(StackFrames, NumFrames, STDERR_FILENO);
  exit(139);
}

void setup_main() { signal(SIGSEGV, handler); }

#else

void setup_main() {}

#endif

int run_game(cxxg::Screen &Scr, const rogue::GameConfig &Cfg) {
  rogue::Game GameInstance(Scr, Cfg);
  try {
    GameInstance.initialize();
    GameInstance.run();
  } catch (std::exception const &E) {
    std::cerr << "ERROR: Running game:" << E.what() << std::endl;
    return 1;
  }
  return 0;
}

int wrapped_main(int Argc, char *Argv[]) {
  if (Argc == 2 && (std::string_view(Argv[1]) == "--help" ||
                    std::string_view(Argv[1]) == "-h")) {
    std::cout << "Usage: " << Argv[0] << " <game_config.json> (--seed <value>)"
              << std::endl;
    return 0;
  }
  std::filesystem::path CfgFile;
  if (Argc >= 2) {
    CfgFile = Argv[1];
  } else {
    CfgFile = std::filesystem::path(Argv[0]).parent_path() / ".." / "shared" /
              "data" / "game_config.json";
  }
  auto Cfg = rogue::GameConfig::load(CfgFile);
  if (Argc == 4) {
    assert(std::string(Argv[2]) == "--seed");
    Cfg.Seed = std::stoi(Argv[3]);
  } else {
    Cfg.Seed = std::time(nullptr);
  }

  // Dump the game configuration and clear the screen
  std::cout << Cfg << "\033[2J";

  cxxg::Screen Scr(cxxg::Screen::getTerminalSize());
  cxxg::utils::registerSigintHandler([]() { exit(0); });

  int Ret = 0;
  while (!Ret) {
    try {
      Ret = run_game(Scr, Cfg);
    } catch (const rogue::GameOverException &) {
    }
    Cfg.Seed = std::time(nullptr);
  }

  return 0;
}

int main(int Argc, char *Argv[]) {
  setup_main();
  try {
    return wrapped_main(Argc, Argv);
  } catch (std::exception const &E) {
    std::cerr << "ERROR: " << E.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "ERROR: Unknown exception" << std::endl;
    return 1;
  }
  return 0;
}