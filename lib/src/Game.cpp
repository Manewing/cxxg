#include <cxxg/Game.h>

#include <cxxg/Utils.h>
#include <stdio.h>

namespace cxxg {

Game::Game() : Scr(Screen::getTerminalSize()) {}

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

void Game::checkSize(ScreenSize GameSize) const {
  // get screen size
  auto const Size = Scr.getSize();

  // check that screen size is sufficient
  if (Size.X < GameSize.X || Size.Y < GameSize.Y) {
    ::std::stringstream SS;
    SS << "Screen size { " << Size.X << ", " << Size.Y << " } is to small for {"
       << GameSize.X << ", " << GameSize.Y << "}";
    throw ::std::out_of_range(SS.str());
  }
}

ScreenSize Game::getOffset(ScreenSize GameSize) const {
  // get screen size and check game size
  auto const Size = Scr.getSize();
  checkSize(GameSize);
  return {(Size.X - GameSize.X) / 2, (Size.Y - GameSize.Y) / 2};
}

}; // namespace cxxg
