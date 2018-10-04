#include <cxxg/Game.h>

#include <cxxg/Utils.h>
#include <stdio.h>

namespace cxxg {

Game::Game(size_t SizeX, size_t SizeY) : Scr(SizeX, SizeY) {}

Game::~Game() {
  if (!utils::hasBufferedInput()) {
    utils::switchBufferedInput();
  }
}

void Game::initialize(bool BufferedInput) {
  if (BufferedInput && utils::hasBufferedInput()) {
    utils::switchBufferedInput();
  }
}

void Game::run() {
  while (GameRunning) {
    handleInput(getchar());
    draw();
  }
}

}; // namespace cxxg
