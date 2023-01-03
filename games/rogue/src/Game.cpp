#include "Game.h"
#include "Components/Player.h"
#include "Components/Transform.h"
#include "Components/Stats.h"
#include "Components/AI.h"
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
    : cxxg::Game(Scr), Hist(*this), EHW(Hist), LevelGen(),
      CurrentLevel(nullptr), UICtrl(Scr) {}

void Game::initialize(bool BufferedInput, unsigned TickDelayUs) {
  EHW.setEventHub(&EvHub);

  switchLevel(0);

  // Player->Inv.Items.push_back(ItemDb.createItem(0, 20));
  // Player->Inv.Items.push_back(ItemDb.createItem(1, 15));
  // Player->Inv.Items.push_back(ItemDb.createItem(2, 10));

  cxxg::Game::initialize(BufferedInput, TickDelayUs);
  handleDraw();
}

void Game::switchLevel(int Level) {
  if (Level < 0) {
    Hist.warn() << "One can never leave...";
    return;
  }

  if (Level >= static_cast<int>(Levels.size())) {
    assert((Level - 1) < static_cast<int>(Levels.size()));
    Levels.push_back(LevelGen.generateLevel(Level));
    Levels.back()->setEventHub(&EvHub);
  }

  if (!CurrentLevel) {
    Levels.at(Level)->createPlayer();
  } else if (CurrentLevel != Levels.at(Level)) {
    Levels.at(Level)->movePlayer(*CurrentLevel);
  }

  CurrentLevel = Levels.at(Level);
  CurrentLevelIdx = Level;
}

bool Game::handleInput(int Char) {
  if (UICtrl.isUIActive()) {
    UICtrl.handleInput(Char);
    return true;
  }

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
  case 'i':
    // UI interaction do not update level
//    UICtrl.setInventoryUI(Player->Inv);
    return true;
  case 'h':
    UICtrl.setHistoryUI(Hist);
    return true;
  default:
    // Not a valid input do not update
    return false;
  }

  // Update level and handle entity updates
  if (!CurrentLevel->update()) {
    // FIXME return false currently indicates player died, refactor for event
    // of player death
  }
  return true;
}

void Game::handleDraw() {
  // Render the current map
  const auto RenderSize = ymir::Size2d<int>{80, 24};
  auto Player = CurrentLevel->getPlayer();
  auto &PC = CurrentLevel->Reg.get<PlayerComp>(Player);
  auto PlayerPos = CurrentLevel->Reg.get<PositionComp>(Player).Pos;
  auto LOSRange = CurrentLevel->Reg.get<LineOfSightComp>(Player).LOSRange;
  auto Health = CurrentLevel->Reg.get<HealthComp>(Player).Health;

  Renderer Render(RenderSize, *CurrentLevel, PlayerPos);
  Render.renderShadow(/*Darkness=*/30);
  Render.renderFogOfWar(CurrentLevel->getPlayerSeenMap());
  Render.renderLineOfSight(PlayerPos, /*Range=*/LOSRange);

  // Draw map
  Scr << Render.get();

  std::string_view InteractStr = "";
  if (PC.CurrentInteraction) {
    InteractStr = PC.CurrentInteraction->Msg;
  } else if (CurrentLevel->canInteract(PlayerPos)) {
    InteractStr = "[E] Interact";
  }

  // Draw UI overlay
  UICtrl.draw(CurrentLevelIdx, Health, InteractStr);

  cxxg::Game::handleDraw();
}

void Game::movePlayer(ymir::Dir2d Dir) {
  if (!CurrentLevel) {
    return;
  }
  auto Player = CurrentLevel->getPlayer();
  if (Player == entt::null) {
    return;
  }
  assert(CurrentLevel->Reg.valid(Player));
  assert(CurrentLevel->Reg.all_of<MovementComp>(Player));
  CurrentLevel->Reg.get<MovementComp>(Player).Dir = Dir;
}

void Game::tryInteract() {

  /*
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
  */
}