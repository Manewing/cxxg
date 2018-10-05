#include <cxxg/Game.h>

#include <cxxg/Utils.h>
#include <stdio.h>

namespace cxxg {

Game::Game(ScreenSize Size) : Scr(Size) {}

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

void Game::draw() {
  for (size_t L = 0; L < Warnings.size(); L++) {
    // get position for warning
    auto Y = Scr.getSize().Y - Warnings.size() + L;

    // print warning
    Scr[Y][0] << ::cxxg::Color::YELLOW << "WARNING: " << Warnings.at(L);
  }
  Warnings.clear();

  Scr.update();
  Scr.clear();
}

void Game::warn(::std::string Warning) {
  Warnings.push_back(::std::move(Warning));
}

}; // namespace cxxg
