#include <cxxg/Game.h>

#include <chrono>
#include <cxxg/Utils.h>

namespace cxxg {

Game::Game(Screen &Scr)
    : Scr(Scr), GameRunning(true), RndEngine(utils::getTimeStamp()) {}

void Game::initialize(bool BufferedInput, unsigned TDU) {
  // switch input if requested input type buffered/un-buffered does not match
  // the current one
  if (BufferedInput != utils::hasBufferedInput()) {
    utils::switchBufferedInput();
  }

  TickDelayUs = TDU;
}

void Game::run(bool Blocking) {
  while (GameRunning) {
    int Char = cxxg::utils::getChar(Blocking);
    handleInput(Char);
    handleDraw();

    if (TickDelayUs) {
      cxxg::utils::sleep(TickDelayUs);
    }
  }
}

void Game::handleDraw() {
  for (size_t L = 0; L < Warnings.size(); L++) {
    // get position for warning
    auto Y = Scr.getSize().Y - Warnings.size() + L;

    // print warning
    Scr[Y] = Warnings.at(L);
  }
  Warnings.clear();

  Scr.update();
  Scr.clear();
}

void Game::handleExit() {
  // set input back to buffered input
  if (!utils::hasBufferedInput()) {
    utils::switchBufferedInput();
  }
}

RowAccessor Game::warn() {
  Warnings.push_back(Row(Scr.getSize().X));
  return (Warnings.back()[0] << ::cxxg::types::Color::YELLOW << "WARNING: ");
}

void Game::checkSize(types::Size GameSize) const {
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

types::Size Game::getOffset(types::Size GameSize) const {
  // get screen size and check game size
  auto const Size = Scr.getSize();
  checkSize(GameSize);
  return {(Size.X - GameSize.X) / 2, (Size.Y - GameSize.Y) / 2};
}

void Game::setRandomSeed(size_t Seed) {
  RndEngine = ::std::default_random_engine(Seed);
}

}; // namespace cxxg
