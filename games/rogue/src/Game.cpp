#include "Game.h"
#include "Renderer.h"
#include <cxxg/Row.h>
#include <cxxg/Screen.h>
#include <cxxg/Types.h>
#include <cxxg/Utils.h>
#include <memory>

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
  Player = std::make_unique<PlayerEntity>();

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
  } else {
    CurrentLevel->setPlayer(nullptr);
  }
  CurrentLevel = Levels.at(Level);

  CurrentLevel->setPlayer(Player.get());
  if (CurrentLevelIdx <= Level) {
    Player->Pos = CurrentLevel->getPlayerStartPos();
  } else {
    Player->Pos = CurrentLevel->getPlayerEndPos();
  }

  CurrentLevelIdx = Level;
}

bool Game::handleInput(int Char) {

  // FIXME play move and attack needs to be handled in update
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
    break;
  default:
    // Not a valid input do not update
    return false;
  }

  // Update level and handle entity updates
  if (!CurrentLevel->update()) {
    warn() << "dead";
  }
  return true;
}

void Game::handleDraw() {
  // Render the current map
  Renderer Render(*CurrentLevel);
  Render.renderShadow(/*Darkness=*/30);
  Render.renderLineOfSight(Player->Pos, /*Range=*/Player->LOSRange);
  // Render.renderLineOfSight({50, 14}, /*Range=*/5);

  // Draw map
  Scr << Render.get();
  auto ScrSize = Scr.getSize();

  // Draw UI overlay
  Scr[0][0] << "Rogue v0.0 [FLOOR]: " << (CurrentLevelIdx + 1)
            << " [HEALTH]: " << Player->Health;

  std::string_view InteractStr = "";
  if (Player->CurrentInteraction) {
    InteractStr = Player->CurrentInteraction->Msg;
  } else if (CurrentLevel->canInteract(Player->Pos)) {
    InteractStr = "[E] Interact";
  }
  if (!InteractStr.empty()) {
    Scr[ScrSize.Y - 2][ScrSize.X / 2 - InteractStr.size() / 2] << InteractStr;
  }

  cxxg::Game::handleDraw();
}

void Game::movePlayer(ymir::Dir2d Dir) {
  // Abort interaction
  if (Player->CurrentInteraction) {
    Player->CurrentInteraction = std::nullopt;
  }
  auto NewPos = Player->Pos + Dir;

  if (auto *Entity = CurrentLevel->getEntityAt(NewPos)) {
    // TODO check if player can attack entity
    std::cerr << "Entity: " << Entity->T.T << ", Health: " << Entity->Health
              << std::endl;
    Player->attackEntity(*Entity);
    return;
  }

  if (!CurrentLevel->isBodyBlocked(NewPos)) {
    Player->Pos = NewPos;
  } else {
    warn() << "Can't move";
  }
}

void Game::tryInteract() {
  // Finalize interaction
  if (Player->CurrentInteraction) {
    Player->CurrentInteraction->Finalize();
    Player->CurrentInteraction = std::nullopt;
    return;
  }

  auto Interactables = CurrentLevel->getInteractables(Player->Pos);
  if (Interactables.empty()) {
    return;
  }

  // TODO allow cycling through available objects
  auto &Interactable = Interactables.at(0);
  switch (Interactable.kind()) {
  case 'H':
    Player->CurrentInteraction = {
        "[E] Previous level", [this]() { switchLevel(CurrentLevelIdx - 1); }};
    break;
  case '<':
    Player->CurrentInteraction = {
        "[E] Next level", [this]() { switchLevel(CurrentLevelIdx + 1); }};
    break;
  case 'C':
    Player->CurrentInteraction = {"[E] Close chest"};
    break;
  }
}