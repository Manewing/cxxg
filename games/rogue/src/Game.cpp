#include "Game.h"
#include <cmath>
#include <cxxg/Row.h>
#include <cxxg/Screen.h>
#include <cxxg/Types.h>
#include "Renderer.h"
#include <cxxg/Utils.h>
#include <gtest/gtest.h>
#include <ymir/Algorithm/LineOfSight.hpp>

template <typename T, typename U>
cxxg::Screen &operator<<(cxxg::Screen &Scr, const ymir::Map<T, U> &Map) {
  for (auto PY = 0; PY < Map.getSize().H; PY++) {
    for (auto PX = 0; PX < Map.getSize().W; PX++) {
      Scr[PY][PX] = Map.getTile({PX, PY});
    }
  }
  return Scr;
}

Game::Game(cxxg::Screen &Scr)
    : cxxg::Game(Scr), LevelGen(), CurrentLevel(nullptr) {}

void Game::initialize(bool BufferedInput, unsigned TickDelayUs) {
  // FIXME
  PlayerPos = {37, 11};

  switchLevel(0);

  cxxg::Game::initialize(BufferedInput, TickDelayUs);
  handleDraw();
}

void Game::switchLevel(int Level) {
  if (Level < 0) {
    warn() << "One can never leave...";
    return;
  }
  if (Level >= static_cast<int>(Levels.size())) {
    assert((Level - 1) < Levels.size());
    Levels.push_back(LevelGen.generateLevel(Level));
  }
  CurrentLevel = Levels.at(Level);

  if (CurrentLevelIdx <= Level) {
    PlayerPos = CurrentLevel->getPlayerStartPos();
  } else {
    PlayerPos = CurrentLevel->getPlayerEndPos();
  }

  CurrentLevelIdx = Level;
}

void Game::handleInput(int Char) {
  // TODO handle entity actions

  //
  switch (Char) {
  case 'a':
  case cxxg::utils::KEY_LEFT:
    movePlayer(ymir::Dir2d::LEFT);
    break;
  case 'd':
  case cxxg::utils::KEY_RIGHT:
    movePlayer(ymir::Dir2d::RIGHT);
    break;
  case 's':
  case cxxg::utils::KEY_DOWN:
    movePlayer(ymir::Dir2d::DOWN);
    break;
  case 'w':
  case cxxg::utils::KEY_UP:
    movePlayer(ymir::Dir2d::UP);
    break;
  case cxxg::utils::KEY_SPACE:
    break;
  case 'e':
    tryInteract();
  default:
    break;
  }
}

void Game::handleDraw() {
  // Render the current map
  Renderer Render(*CurrentLevel, PlayerPos);
  Render.renderShadow(/*Darkness=*/30);
  Render.renderLineOfSight(PlayerPos, /*Range=*/8);
  // Render.renderLineOfSight({50, 14}, /*Range=*/5);

  // Draw map
  Scr << Render.get();
  auto ScrSize = Scr.getSize();

  // Draw UI overlay
  Scr[0][0] << "Rogue v0.0 [Level]: " << (CurrentLevelIdx + 1);

  std::string_view InteractStr = "";
  if (CurrentInteraction) {
    InteractStr = CurrentInteraction->Msg;
  } else if (CurrentLevel->canInteract(PlayerPos)) {
    InteractStr = "[E] Interact";
  }
  if (!InteractStr.empty()) {
    Scr[ScrSize.Y - 2][ScrSize.X / 2 - InteractStr.size() / 2] << InteractStr;
  }

  cxxg::Game::handleDraw();
}

void Game::movePlayer(ymir::Dir2d Dir) {
  // Abort interaction
  if (CurrentInteraction) {
    CurrentInteraction = std::nullopt;
  }

  auto NewPos = PlayerPos + Dir;
  if (!CurrentLevel->isBodyBlocked(NewPos)) {
    PlayerPos += Dir;
  } else {
    warn() << "Can't move";
  }
}

void Game::tryInteract() {
  // Finalize interaction
  if (CurrentInteraction) {
    CurrentInteraction->Finalize();
    CurrentInteraction = std::nullopt;
    return;
  }

  auto Interactables = CurrentLevel->getInteractables(PlayerPos);
  if (Interactables.empty()) {
    return;
  }

  // TODO allow cycling through available objects
  auto &Interactable = Interactables.at(0);
  switch (Interactable.kind()) {
  case 'H':
    CurrentInteraction = {"[E] Previous level",
                          [this]() { switchLevel(CurrentLevelIdx - 1); }};
    break;
  case '<':
    CurrentInteraction = {"[E] Next level",
                          [this]() { switchLevel(CurrentLevelIdx + 1); }};
    break;
  case 'C':
    CurrentInteraction = {"[E] Close chest"};
    break;
  }
}