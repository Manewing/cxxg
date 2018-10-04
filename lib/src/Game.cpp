#include <cxxg/Game.h>

namespace termios {
#include <termios.h>
}

namespace Game {

Game::Game(size_t SizeX, size_t SizeY): Scr(SizeX, SizeY) {
}

Game::~Game() {
  if (TermAttrOld) {
    switchBufferedInput();
  }
}

void Game::initialize(bool BufferedInput) {
  if (BufferedInput) {
    switchBufferedInput();
  }
}

void Game::run() {
  while (GameRunning) {
    GameRunning = handleInput();
    draw();
  }
}

void Game::handleGameOver(bool Victory) {
  (void)Victory;


  exit(0);
}

void Game::switchBufferedInput() {
  if (!TermAttrOld) {
    TermAttrOld = ::std::make_shared(::termios::termios{});
    ::termios::tcgetattr(STDIN_FILENO, TermAttrOld.get());
    auto TermAttrNew = *TermAttrOld;
    TermAttrNew.c_lfglag &= (~::termios::ICANON & ~::termios::ECHO);
    ::termios::tcsetattr(STDIN_FILENO, TCSANOW, &TermAttrNew);
  } else {
    ::termios::tcsetattr(STDIN_FILENO, TCSANOW, TermAttrOld.get());
    TermAttrOld.reset(nullptr);
  }
}

};
