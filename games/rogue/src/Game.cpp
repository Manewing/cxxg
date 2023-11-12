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
      Ctx({ItemDb, CreatureDb}),
      LvlGen(LevelGeneratorLoader(Ctx).load(Cfg.Seed, Cfg.InitialLevelConfig)),
      World(GameWorld::create(*LvlGen, Cfg.InitialGameWorld)), UICtrl(Scr) {}

namespace {

void fillPlayerInventory(entt::registry &Reg, entt::entity Player,
                         const GameConfig &Cfg, const ItemDatabase &ItemDb) {
  auto &InvComp = Reg.get<InventoryComp>(Player);
  auto &EquipComp = Reg.get<EquipmentComp>(Player);
  auto &Inv = InvComp.Inv;
  auto &Equip = EquipComp.Equip;

  // FIXME this should be part of an enemy/NPC AI system
  for (const auto &ItCfg : Cfg.InitialItems) {
    auto ItId = ItemDb.getItemId(ItCfg.Name);
    auto It = ItemDb.createItem(ItId, ItCfg.Count);
    Inv.addItem(It);
  }

  // FIXME this should be part of an enemy/NPC AI system
  // Try equipping items
  for (std::size_t Idx = 0; Idx < Inv.size(); Idx++) {
    const auto &It = Inv.getItem(Idx);
    if (Equip.canEquip(It, Player, Reg)) {
      Equip.equip(Inv.takeItem(Idx), Player, Reg);
      Idx -= 1;
    }
  }
}

} // namespace

void Game::initialize(bool BufferedInput, unsigned TickDelayUs) {
  EHW.setEventHub(&EvHub);
  EHW.subscribe(*this, &Game::onEntityDiedEvent);
  EHW.subscribe(*this, &Game::onSwitchLevelEvent);
  EHW.subscribe(*this, &Game::onSwitchGameWorldEvent);
  EHW.subscribe(*this, &Game::onLootEvent);
  REC.setEventHub(&EvHub);
  World->setEventHub(&EvHub);
  UICtrl.setEventHub(&EvHub);

  switchLevel(0, /*ToEntry=*/true);

  // Fill player inventory
  World->getCurrentLevelOrFail().createPlayer();
  fillPlayerInventory(getLvlReg(), getPlayer(), Cfg, ItemDb);

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

  auto *CurrLvl = World->getCurrentLevel();
  auto &Nextlvl = World->switchLevel(Level);

  if (CurrLvl && CurrLvl != &Nextlvl) {
    auto ToPos =
        ToEntry ? Nextlvl.getPlayerStartPos() : Nextlvl.getPlayerEndPos();
    Nextlvl.update(false);
    Nextlvl.movePlayer(*CurrLvl, ToPos);
  }
}

bool Game::handleInput(int Char) {
  movePlayer(ymir::Dir2d::NONE);

  // Override keys
  switch (Char) {
  case ui::Controls::InventoryUI.Char: {
    handleUpdates(/*IsTick=*/false);
    if (UICtrl.hasInventoryUI()) {
      UICtrl.closeInventoryUI();
      handleUpdates(/*IsTick=*/!UICtrl.isUIActive());
    } else {
      UICtrl.setInventoryUI(getPlayerOrNull(), getLvlReg());
    }
    return true;
  }
  case ui::Controls::EquipmentUI.Char: {
    handleUpdates(/*IsTick=*/false);
    if (UICtrl.hasEquipmentUI()) {
      UICtrl.closeEquipmentUI();
      handleUpdates(/*IsTick=*/!UICtrl.isUIActive());
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
                         World->getCurrentLevelOrFail());
    }
    return true;
  }
  case 'l':
    switchLevel(World->getCurrentLevelIdx() + 1, true);
    break;

  // TODO show controls
  default:
    break;
  }

  // Handle UI input
  if (UICtrl.isUIActive()) {
    UICtrl.handleInput(Char);
    handleUpdates(/*IsTick=*/!UICtrl.isUIActive());
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
    // Wait a turn
    Hist.info() << "Resting...";
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
    World->getCurrentLevelOrFail().update(IsTick);
    return true;
  }

  // While the game is running update the level and draw to screen.
  // We will perform ticks until enough ticks have passed for the player to have
  // gained enough AP to take an action.
  while (GameRunning) {
    World->getCurrentLevelOrFail().update(true);
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
  auto Player = getPlayer();
  getLvlReg().get<PlayerComp>(Player).MoveDir = Dir;
}

// FIXME move to player system?
void Game::tryInteract() {
  if (auto *Interact = getAvailableInteraction()) {
    auto Player = getPlayer();
    auto &PC = getLvlReg().get<PlayerComp>(Player);
    Interact->Execute(World->getCurrentLevelOrFail(), Player, getLvlReg());
    PC.CurrentInteraction = *Interact;
  }
}

entt::registry &Game::getLvlReg() { return World->getCurrentLevelOrFail().Reg; }

entt::entity Game::getPlayerOrNull() const {
  const auto *CurrentLevel = World->getCurrentLevel();
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

  const auto &CurrentLevel = World->getCurrentLevelOrFail();
  auto InteractableEntities = CurrentLevel.getInteractables(PlayerPos);
  if (InteractableEntities.empty()) {
    return nullptr;
  }

  // TODO allow cycling through available objects
  auto &InteractableEntity = InteractableEntities.at(0);
  auto &Interactable = getLvlReg().get<InteractableComp>(InteractableEntity);
  return &Interactable.Action;
}

void Game::onEntityDiedEvent(const EntityDiedEvent &E) {
  GameRunning = !E.isPlayerAffected();
}

void Game::onSwitchLevelEvent(const SwitchLevelEvent &E) {
  switchLevel(E.Level, E.ToEntry);
}

void Game::onSwitchGameWorldEvent(const SwitchGameWorldEvent &E) {
  REC.clear();

  auto *CurrLvl = World->getCurrentLevel();
  World->switchWorld(Cfg.Seed, E.GameWorldType, E.LevelConfig);
  auto &Nextlvl = World->switchLevel(0);

  if (CurrLvl && CurrLvl != &Nextlvl) {
    auto ToPos = Nextlvl.getPlayerStartPos();
    Nextlvl.update(false);
    Nextlvl.movePlayer(*CurrLvl, ToPos);
  }

  // We could update the level here, but we want to draw the initial state.
  handleUpdates(/*IsTick=*/false);
}

void Game::onLootEvent(const LootEvent &E) {
  if (!E.isPlayerAffected() || !E.Registry) {
    return;
  }
  UICtrl.setLootUI(E.Entity, E.LootedEntity, *E.Registry);
}

namespace {

ui::Controller::PlayerInfo getUIPlayerInfo(entt::entity Player,
                                           entt::registry &Reg,
                                           Interaction *Interact) {
  const auto &Health = Reg.get<HealthComp>(Player);
  const auto &AGC = Reg.get<AgilityComp>(Player);

  ui::Controller::PlayerInfo PI;
  PI.Health = Health.Value;
  PI.MaxHealth = Health.MaxValue;
  PI.AP = AGC.AP;
  if (Interact) {
    PI.InteractStr = "[E] " + Interact->Msg;
  }

  return PI;
}

ui::Controller::TargetInfo getUITargetInfo(entt::entity Target,
                                           entt::registry &Reg) {
  if (Target == entt::null || !Reg.valid(Target)) {
    return {};
  }
  ui::Controller::TargetInfo TI;
  auto *TNC = Reg.try_get<NameComp>(Target);
  auto *THC = Reg.try_get<HealthComp>(Target);
  TI.Name = TNC ? TNC->Name : "<NameCompMissing>";
  TI.Health = THC ? THC->Value : 0;
  TI.MaxHealth = THC ? THC->MaxValue : 0;
  return TI;
}

} // namespace

void Game::handleDrawLevel(bool UpdateScreen) {
  // Render the current map
  const auto RenderSize = ymir::Size2d<int>{static_cast<int>(Scr.getSize().X),
                                            static_cast<int>(Scr.getSize().Y)};

  // Get components for drawing from the current player
  auto Player = getPlayer();
  const auto &PC = getLvlReg().get<PlayerComp>(Player);
  auto PlayerPos = getLvlReg().get<PositionComp>(Player).Pos;
  const auto &LOSRange = getLvlReg().get<LineOfSightComp>(Player).LOSRange;

  auto &CurrentLevel = World->getCurrentLevelOrFail();
  Renderer Render(RenderSize, CurrentLevel, PlayerPos);
  Render.renderShadow(/*Darkness=*/30);
  Render.renderFogOfWar(CurrentLevel.getPlayerSeenMap());
  Render.renderLineOfSight(PlayerPos, /*Range=*/LOSRange);
  REC.apply(Render);
  REC.clear();

  // Draw map
  Scr << Render.get();

  // Draw UI overlay
  auto PI = getUIPlayerInfo(Player, getLvlReg(), getAvailableInteraction());
  auto TI = getUITargetInfo(PC.Target, getLvlReg());
  UICtrl.draw(World->getCurrentLevelIdx(), PI, TI);

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