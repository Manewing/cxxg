#include <cxxg/Row.h>
#include <cxxg/Screen.h>
#include <cxxg/Types.h>
#include <cxxg/Utils.h>
#include <memory>
#include <rogue/Components/AI.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Game.h>
#include <rogue/Renderer.h>

namespace rogue {

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
    : cxxg::Game(Scr), Hist(*this), EHW(Hist), ItemDb(), Ctx({*this, ItemDb}),
      LevelGen(&Ctx), CurrentLevel(nullptr), UICtrl(Scr) {}

void Game::initialize(bool BufferedInput, unsigned TickDelayUs) {
  EHW.setEventHub(&EvHub);

  switchLevel(0);

  // DEBUG ==>
  auto Player = CurrentLevel->getPlayer();
  auto &InvComp = CurrentLevel->Reg.get<InventoryComp>(Player);
  InvComp.Inv.addItem(ItemDb.createItem(0, 20));
  InvComp.Inv.addItem(ItemDb.createItem(1, 15));
  InvComp.Inv.addItem(ItemDb.createItem(2, 10));
  // <== DEBUG

  cxxg::Game::initialize(BufferedInput, TickDelayUs);

  // We could update the level here, but we want to draw the initial state.
  handleUpdates(/*IsTick=*/false);

  handleDraw();
}

void Game::switchLevel(int Level) {
  if (Level < 0) {
    Hist.warn() << "One can never leave...";
    return;
  }

  if (Level >= static_cast<int>(Levels.size())) {
    assert((Level - 1) < static_cast<int>(Levels.size()));
    Levels.push_back(LevelGen.generateLevel(Level, Level));
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

  // Override keys
  switch (Char) {
  case 'i': {
    auto Player = getPlayerOrNull();
    if (Player == entt::null) {
      return false;
    }
    if (UICtrl.hasInventoryUI()) {
      UICtrl.closeInventoryUI();
      return true;
    }
    auto &InvComp = getLvlReg().get<InventoryComp>(Player);
    // UI interaction do not update level
    UICtrl.setInventoryUI(InvComp.Inv, Player, CurrentLevel->Reg);
    return true;
  }
  case 'o': {
    auto Player = getPlayerOrNull();
    if (Player == entt::null) {
      return false;
    }
    if (UICtrl.hasEquipmentUI()) {
      UICtrl.closeEquipmentUI();
      return true;
    }
    auto &EquipComp = getLvlReg().get<EquipmentComp>(Player);
    // UI interaction do not update level
    UICtrl.setEquipmentUI(EquipComp.Equip, Player, CurrentLevel->Reg);
    return true;
  }
  case 'h':
    UICtrl.setHistoryUI(Hist);
    return true;
  case 'c':
    // TODO help show controls
    return false;
  default:
    break;
  }

  // Handle UI input
  if (UICtrl.isUIActive()) {
    UICtrl.handleInput(Char);
    handleUpdates(/*IsTick=*/false);
    return true;
  }

  // FIXME add user input to context and process input in player system
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
    // Does not count as a tick
    return true;
  default:
    // Not a valid input do not update
    return false;
  }

  handleUpdates(/*IsTick=*/true);

  return true;
}

bool Game::handleUpdates(bool IsTick) {
  if (IsTick) {
    GameTicks++;
  }
  if (!CurrentLevel->update(IsTick)) {
    // FIXME return false currently indicates player died, refactor for event
    // of player death
    return false;
  }
  return true;
}

void Game::handleDraw() {
  // Render the current map
  const auto RenderSize = ymir::Size2d<int>{80, 24};

  // FIXME need to check that player is still alive!
  auto Player = getPlayer();
  auto PlayerPos = getLvlReg().get<PositionComp>(Player).Pos;
  auto LOSRange = getLvlReg().get<LineOfSightComp>(Player).LOSRange;
  auto Health = getLvlReg().get<HealthComp>(Player);

  Renderer Render(RenderSize, *CurrentLevel, PlayerPos);
  Render.renderShadow(/*Darkness=*/30);
  Render.renderFogOfWar(CurrentLevel->getPlayerSeenMap());
  Render.renderLineOfSight(PlayerPos, /*Range=*/LOSRange);

  // Draw map
  Scr << Render.get();

  std::string_view InteractStr = "";
  if (auto *Interact = getAvailableInteraction()) {
    InteractStr = "[E] " + Interact->Msg;
  }

  // Draw UI overlay
  UICtrl.draw(CurrentLevelIdx, Health.Value, Health.MaxValue, InteractStr);

  /// DEBUG==>
  // const auto &SC = CurrentLevel->Reg.get<StatsComp>(Player);
  // Scr[1][0] << "Int: " << SC.effective().Int << ", Str: " <<
  // SC.effective().Str
  //           << ", Dex: " << SC.effective().Dex
  //           << ", Vit: " << SC.effective().Vit;
  /// <== DEBUG

  cxxg::Game::handleDraw();
}

// FIXME move to player system?
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

// FIXME move to player system?
void Game::tryInteract() {
  if (auto *Interact = getAvailableInteraction()) {
    auto Player = getPlayer();
    auto &PC = getLvlReg().get<PlayerComp>(Player);
    Interact->Execute(*this, Player, getLvlReg());
    PC.CurrentInteraction = *Interact;
  }
}

entt::registry &Game::getLvlReg() {
  assert(CurrentLevel); // FIXME this should be an exception
  return CurrentLevel->Reg;
}

entt::entity Game::getPlayerOrNull() const {
  if (!CurrentLevel) {
    return entt::null;
  }
  return CurrentLevel->getPlayer();
}

entt::entity Game::getPlayer() const {
  auto Player = getPlayerOrNull();
  assert(Player != entt::null); // FIXME this should be an exception
  return Player;
}

Interaction *Game::getAvailableInteraction() {
  auto Player = getPlayer();
  auto PlayerPos = getLvlReg().get<PositionComp>(Player).Pos;

  auto InteractableEntities = CurrentLevel->getInteractables(PlayerPos);
  if (InteractableEntities.empty()) {
    return nullptr;
  }

  // TODO allow cycling through available objects
  auto &InteractableEntity = InteractableEntities.at(0);
  auto &Interactable =
      CurrentLevel->Reg.get<InteractableComp>(InteractableEntity);
  return &Interactable.Action;
}

} // namespace rogue