#include <cxxg/Row.h>
#include <cxxg/Screen.h>
#include <cxxg/Types.h>
#include <cxxg/Utils.h>
#include <memory>
#include <rogue/Components/AI.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Event.h>
#include <rogue/Game.h>
#include <rogue/GameConfig.h>
#include <rogue/Renderer.h>
#include <rogue/UI/Controls.h>

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

void RenderEventCollector::setEventHub(EventHub *EH) {
  EventHubConnector::setEventHub(EH);
  EH->subscribe(*this, &RenderEventCollector::onEntityAttackEvent);
  EH->subscribe(*this, &RenderEventCollector::onDetectTargetEvent);
  EH->subscribe(*this, &RenderEventCollector::onLostTargetEvent);
}

void RenderEventCollector::onEntityAttackEvent(const EntityAttackEvent &E) {
  auto *PC = E.Registry->try_get<PositionComp>(E.Target);
  if (!PC) {
    return;
  }
  auto const AtPos = PC->Pos;
  RenderFns.push_back([AtPos](Renderer &R) {
    R.renderEffect(
        cxxg::types::ColoredChar{'*', cxxg::types::RgbColor{155, 20, 20}},
        AtPos);
  });
}

void RenderEventCollector::onDetectTargetEvent(const DetectTargetEvent &E) {
  auto *PC = E.Registry->try_get<PositionComp>(E.Entity);
  if (!PC) {
    return;
  }
  auto const AtPos = PC->Pos;
  RenderFns.push_back([AtPos](Renderer &R) {
    R.renderEffect(
        cxxg::types::ColoredChar{'!', cxxg::types::RgbColor{173, 161, 130}},
        AtPos);
  });
}

void RenderEventCollector::onLostTargetEvent(const LostTargetEvent &E) {
  auto *PC = E.Registry->try_get<PositionComp>(E.Entity);
  if (!PC) {
    return;
  }
  auto const AtPos = PC->Pos;
  RenderFns.push_back([AtPos](Renderer &R) {
    R.renderEffect(
        cxxg::types::ColoredChar{'?', cxxg::types::RgbColor{56, 55, 89}},
        AtPos);
  });
}

void RenderEventCollector::apply(Renderer &R) {
  for (auto &Fn : RenderFns) {
    Fn(R);
  }
}

void RenderEventCollector::clear() { RenderFns.clear(); }

bool RenderEventCollector::hasEvents() const { return !RenderFns.empty(); }

Game::Game(cxxg::Screen &Scr, const GameConfig &Cfg)
    : cxxg::Game(Scr), Cfg(Cfg), Hist(*this), EHW(Hist),
      ItemDb(ItemDatabase::load(Cfg.ItemDbConfig)),
      CreatureDb(CreatureDatabase::load(Cfg.CreatureDbConfig)),
      Ctx({*this, ItemDb, CreatureDb}), LevelGen(&Ctx), CurrentLevel(nullptr),
      UICtrl(Scr) {}

void Game::initialize(bool BufferedInput, unsigned TickDelayUs) {
  EHW.setEventHub(&EvHub);
  EHW.subscribe(*this, &Game::onEntityDiedEvent);
  EHW.subscribe(*this, &Game::onSwitchLevelEvent);
  EHW.subscribe(*this, &Game::onLootEvent);
  REC.setEventHub(&EvHub);
  UICtrl.setEventHub(&EvHub);

  switchLevel(0, /*ToEntry=*/true);

  // Fill player inventory
  auto Player = CurrentLevel->getPlayer();
  auto &InvComp = CurrentLevel->Reg.get<InventoryComp>(Player);
  for (const auto &ItCfg : Cfg.InitialItems) {
    auto ItId = ItemDb.getItemId(ItCfg.Name);
    auto It = ItemDb.createItem(ItId, ItCfg.Count);
    InvComp.Inv.addItem(It);
  }

  cxxg::Game::initialize(BufferedInput, TickDelayUs);

  // We could update the level here, but we want to draw the initial state.
  handleUpdates(/*IsTick=*/false);

  handleDraw();
}

void Game::switchLevel(int Level, bool ToEntry) {
  if (Level < 0) {
    Hist.warn() << "One can never leave...";
    return;
  }

  REC.clear();

  if (Level >= static_cast<int>(Levels.size())) {
    assert((Level - 1) < static_cast<int>(Levels.size()));
    const auto &LvlCfg = Cfg.getLevelRangeConfig(Level);
    Levels.push_back(
        LevelGen.generateLevel(Cfg.Seed + Level, Level, LvlCfg.Config));
    Levels.back()->setEventHub(&EvHub);
  }

  const auto &LevelPtr = Levels.at(Level);

  if (!CurrentLevel) {
    LevelPtr->createPlayer();
  } else if (CurrentLevel != LevelPtr) {
    auto ToPos =
        ToEntry ? LevelPtr->getPlayerStartPos() : LevelPtr->getPlayerEndPos();
    LevelPtr->update(false);
    LevelPtr->movePlayer(*CurrentLevel, ToPos);
  }

  CurrentLevel = LevelPtr;
  CurrentLevelIdx = Level;
}

bool Game::handleInput(int Char) {

  // Override keys
  switch (Char) {
  case ui::Controls::InventoryUI.Char: {
    handleUpdates(/*IsTick=*/false);
    if (UICtrl.hasInventoryUI()) {
      UICtrl.closeInventoryUI();
    } else {
      UICtrl.setInventoryUI(getPlayerOrNull(), getLvlReg());
    }
    return true;
  }
  case ui::Controls::EquipmentUI.Char: {
    handleUpdates(/*IsTick=*/false);
    if (UICtrl.hasEquipmentUI()) {
      UICtrl.closeEquipmentUI();
    } else {
      UICtrl.setEquipmentUI(getPlayerOrNull(), getLvlReg());
    }
    return true;
  }
  case ui::Controls::BuffsUI.Char: {
    handleUpdates(/*IsTick=*/false);
    if (UICtrl.hasBuffUI()) {
      UICtrl.closeBuffUI();
    } else {
      UICtrl.setBuffUI(getPlayerOrNull(), getLvlReg());
    }
    return true;
  }
  case ui::Controls::HistoryUI.Char:
    handleUpdates(/*IsTick=*/false);
    if (UICtrl.hasHistoryUI()) {
      UICtrl.closeHistoryUI();
    } else {
      UICtrl.setHistoryUI(Hist);
    }
    return true;
  case ui::Controls::CharacterUI.Char: {
    handleUpdates(/*IsTick=*/false);
    if (UICtrl.hasStatsUI()) {
      UICtrl.closeStatsUI();
    } else {
      UICtrl.setStatsUI(getPlayerOrNull(), getLvlReg());
    }
    return true;
  }
  case ui::Controls::TargetUI.Char: {
    handleUpdates(/*IsTick=*/false);
    if (UICtrl.hasTargetUI()) {
      UICtrl.closeTargetUI();
    } else {
      UICtrl.setTargetUI(getPlayer(),
                         getLvlReg().get<PositionComp>(getPlayer()),
                         *CurrentLevel);
    }
    return true;
  }
  case 'l':
    switchLevel(CurrentLevelIdx + 1, true);
    break;

  // TODO show controls
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
    movePlayer(ymir::Dir2d::NONE);
    break;
  case 'e':
    tryInteract();
    // Does not count as a tick
    return true;
  default:
    // Not a valid input do not update
    return true;
  }

  return handleUpdates(/*IsTick=*/true);
}

bool Game::handleUpdates(bool IsTick) {
  if (!IsTick) {
    CurrentLevel->update(IsTick);
    return true;
  }

  // While the game is running update the level and draw to screen.
  // We will perform ticks until enough ticks have passed for the player to have
  // gained enough AP to take an action.
  while (GameRunning) {
    CurrentLevel->update(true);
    GameTicks++;

    if (!GameRunning) {
      break;
    }

    auto SleepAfterDraw = REC.hasEvents();
    handleDrawLevel(true);
    if (SleepAfterDraw) {
      cxxg::utils::sleep(200000);
    }

    // Clear stdin buffer
    cxxg::utils::getChar(false);

    auto PC = getLvlReg().get<PlayerComp>(getPlayer());
    if (PC.IsReady) {
      break;
    }
  }

  return true;
}

void Game::handleDraw() {
  if (GameRunning) {
    handleDrawLevel(false);
  } else {
    handleDrawGameOver();
  }

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
  assert(CurrentLevel->Reg.all_of<PlayerComp>(Player));
  CurrentLevel->Reg.get<PlayerComp>(Player).MoveDir = Dir;
}

// FIXME move to player system?
void Game::tryInteract() {
  if (auto *Interact = getAvailableInteraction()) {
    auto Player = getPlayer();
    auto &PC = getLvlReg().get<PlayerComp>(Player);
    Interact->Execute(*CurrentLevel, Player, getLvlReg());
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

void Game::onEntityDiedEvent(const EntityDiedEvent &E) {
  GameRunning = !E.isPlayerAffected();
}

void Game::onSwitchLevelEvent(const SwitchLevelEvent &E) {
  switchLevel(E.Level, E.ToEntry);
}

void Game::onLootEvent(const LootEvent &E) {
  if (!E.isPlayerAffected() || !E.Registry) {
    return;
  }
  UICtrl.setLootUI(E.Entity, E.LootedEntity, *E.Registry);
}

void Game::handleDrawLevel(bool UpdateScreen) {
  // Render the current map
  const auto RenderSize = ymir::Size2d<int>{static_cast<int>(Scr.getSize().X),
                                            static_cast<int>(Scr.getSize().Y)};

  // Get components for drawing from the current player
  auto Player = getPlayer();
  const auto &PC = getLvlReg().get<PlayerComp>(Player);
  auto PlayerPos = getLvlReg().get<PositionComp>(Player).Pos;
  const auto &LOSRange = getLvlReg().get<LineOfSightComp>(Player).LOSRange;
  const auto &Health = getLvlReg().get<HealthComp>(Player);
  const auto &AGC = getLvlReg().get<AgilityComp>(Player);

  ui::Controller::PlayerInfo PI;
  PI.Health = Health.Value;
  PI.MaxHealth = Health.MaxValue;
  PI.AP = AGC.AP;
  if (auto *Interact = getAvailableInteraction()) {
    PI.InteractStr = "[E] " + Interact->Msg;
  }

  std::optional<ui::Controller::TargetInfo> TI;
  if (PC.Target != entt::null && getLvlReg().valid(PC.Target)) {
    TI = ui::Controller::TargetInfo();
    auto *TNC = getLvlReg().try_get<NameComp>(PC.Target);
    auto *THC = getLvlReg().try_get<HealthComp>(PC.Target);
    TI->Name = TNC ? TNC->Name : "<NameCompMissing>";
    TI->Health = THC ? THC->Value : 0;
    TI->MaxHealth = THC ? THC->MaxValue : 0;
  }

  Renderer Render(RenderSize, *CurrentLevel, PlayerPos);
  Render.renderShadow(/*Darkness=*/30);
  Render.renderFogOfWar(CurrentLevel->getPlayerSeenMap());
  Render.renderLineOfSight(PlayerPos, /*Range=*/LOSRange);
  REC.apply(Render);
  REC.clear();

  // Draw map
  Scr << Render.get();

  // Draw UI overlay
  UICtrl.draw(CurrentLevelIdx, PI, TI);

  if (UpdateScreen) {
    handleShowNotifications(false);
    Scr.update();
    Scr.clear();
  }
}

void Game::handleDrawGameOver() {
  // TODO
}

} // namespace rogue