#include <cxxg/Row.h>
#include <cxxg/Screen.h>
#include <cxxg/Types.h>
#include <cxxg/Utils.h>
#include <memory>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/LOS.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Event.h>
#include <rogue/Game.h>
#include <rogue/GameConfig.h>
#include <rogue/InventoryHandler.h>
#include <rogue/Renderer.h>
#include <rogue/UI/CommandLine.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/Equipment.h>
#include <rogue/UI/TargetUI.h>

namespace rogue {

Game::Game(cxxg::Screen &Scr, const GameConfig &Cfg)
    : cxxg::Game(Scr), Cfg(Cfg), Hist(*this), EHW(Hist),
      ItemDb(ItemDatabase::load(Cfg.ItemDbConfig)),
      EntityDb(EntityDatabase::load(ItemDb, Cfg.EntityDbConfig)),
      LevelDb(LevelDatabase::load(Cfg.LevelDbConfig)),
      CraftingDb(CraftingDatabase::load(ItemDb, Cfg.CraftingDbConfig)),
      Crafter(ItemDb),
      Ctx({EvHub, ItemDb, EntityDb, LevelDb, CraftingDb, Crafter}),
      LvlGen(LevelGeneratorLoader(Ctx).load(Cfg.Seed, Cfg.InitialLevelConfig)),
      World(GameWorld::create(LevelDb, *LvlGen, Cfg.InitialGameWorld)),
      UICtrl(Scr) {
  for (const auto &[RecipeId, Recipe] : CraftingDb.getRecipes()) {
    Crafter.addRecipe(RecipeId, Recipe);
  }
}

namespace {

void fillPlayerInventory(entt::registry &Reg, entt::entity Player,
                         const GameConfig &Cfg, const ItemDatabase &ItemDb,
                         const CraftingHandler &Crafter) {
  // FIXME this should be part of an enemy/NPC AI system
  auto &Inv = Reg.get<InventoryComp>(Player).Inv;
  for (const auto &ItCfg : Cfg.InitialItems) {
    auto ItId = ItemDb.getItemId(ItCfg.Name);
    auto It = ItemDb.createItem(ItId, ItCfg.Count);
    Inv.addItem(It);
  }

  // Try equipping items
  InventoryHandler InvHandler(Player, Reg, Crafter);
  InvHandler.autoEquipItems();
}

} // namespace

void Game::initialize(bool BufferedInput, unsigned TickDelayUs) {
  // Set seed for random number generator
  std::srand(Cfg.Seed);

  EHW.setEventHub(&EvHub);
  EHW.subscribe(*this, &Game::onEntityDiedEvent);
  EHW.subscribe(*this, &Game::onSwitchLevelEvent);
  EHW.subscribe(*this, &Game::onSwitchGameWorldEvent);
  EHW.subscribe(*this, &Game::onLootEvent);
  EHW.subscribe(*this, &Game::onCraftEvent);
  REC.setEventHub(&EvHub);
  World->setEventHub(&EvHub);
  UICtrl.setEventHub(&EvHub);

  switchLevel(0, /*ToEntry=*/true);

  // Fill player inventory
  World->getCurrentLevelOrFail().createPlayer();
  fillPlayerInventory(getLvlReg(), getPlayer(), Cfg, ItemDb, Crafter);

  cxxg::Game::initialize(BufferedInput, TickDelayUs);

  // We could update the level here, but we want to draw the initial state.
  handleUpdates(/*IsTick=*/false);

  UICtrl.setMenuUI(World->getCurrentLevelOrFail());

  handleDraw();
}

void Game::switchLevel(int Level, bool ToEntry) {
  if (Level < 0) {
    Hist.warn() << "One can never leave...";
    return;
  }

  UICtrl.closeAll();
  REC.clear();
  World->switchLevel(Level, ToEntry);
}

bool Game::handleInput(int Char) {
  if (UICtrl.hasCommandLineUI()) {
    UICtrl.getWindowOfType<ui::CommandLineController>()->handleInput(Char);
    return true;
  }
  Char = ui::Controls::getRemappedChar(Char);

  movePlayer(ymir::Dir2d::NONE);

  // Override keys
  switch (Char) {
  case ui::Controls::InventoryUI.Char: {
    handleUpdates(/*IsTick=*/false);
    if (UICtrl.hasInventoryUI()) {
      UICtrl.closeInventoryUI();
      handleUpdates(/*IsTick=*/!UICtrl.isUIActive());
    } else {
      UICtrl.setInventoryUI(getPlayerOrNull(), World->getCurrentLevelOrFail());
    }
    return true;
  }
  case ui::Controls::EquipmentUI.Char: {
    handleUpdates(/*IsTick=*/false);
    if (UICtrl.hasEquipmentUI()) {
      UICtrl.closeEquipmentUI();
      handleUpdates(/*IsTick=*/!UICtrl.isUIActive());
    } else {
      UICtrl.setEquipmentUI(getPlayerOrNull(), World->getCurrentLevelOrFail());
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
      auto &Lvl = World->getCurrentLevelOrFail();
      auto SrcEt = getPlayer();
      const auto &PC = getLvlReg().get<PositionComp>(getPlayer());
      // No range, allow investigate entire level
      UICtrl.setTargetUI(PC, {}, Lvl, [&Lvl, SrcEt](auto TgEt, auto TPos) {
        CombatActionComp CC;
        CC.Target = TgEt;
        CC.RangedPos = TPos;
        Lvl.Reg.template emplace<CombatActionComp>(SrcEt, CC);
      });
    }
    return true;
  }
  case ui::Controls::CloseWindow.Char: {
    if (!UICtrl.isUIActive()) {
      UICtrl.setMenuUI(World->getCurrentLevelOrFail());
      return true;
    } else if (UICtrl.hasMenuUI()) {
      UICtrl.closeMenuUI();
      return true;
    }
    break;
  }
  default:
    break;
  }

  // Handle UI input
  if (UICtrl.isUIActive()) {
    UICtrl.handleInput(Char);
    handleUpdates(/*IsTick=*/!UICtrl.isUIActive());
    return true;
  }

  if (Char >= '0' && Char <= '9') {
    auto Idx = Char == '0' ? 10 : Char - '1';
    auto Player = getPlayer();
    auto &EC = getLvlReg().get<EquipmentComp>(Player);

    const auto IsSlotValid = std::size_t(Idx) < EC.Equip.all().size();
    if (!IsSlotValid) {
      return true;
    }

    if (EC.Equip.all().at(Idx)->It) {
      if (ui::EquipmentController::handleUseSkill(
              UICtrl, World->getCurrentLevelOrFail(), Player,
              *EC.Equip.all().at(Idx))) {
        return handleUpdates(/*IsTick=*/true);
      }
    } else {
      Hist.warn() << "No item equipped in slot "
                  << EC.Equip.all().at(Idx)->BaseTypeFilter;
      handleUpdates(/*IsTick=*/false);
      return true;
    }
  }

  // FIXME add user input to context and process input in player system
  // FIXME play move and attack needs to be handled in update
  switch (Char) {
  case ui::Controls::MoveLeft.Char:
    movePlayer(ymir::Dir2d::LEFT);
    break;
  case ui::Controls::MoveRight.Char:
    movePlayer(ymir::Dir2d::RIGHT);
    break;
  case ui::Controls::MoveDown.Char:
    movePlayer(ymir::Dir2d::DOWN);
    break;
  case ui::Controls::MoveUp.Char:
    movePlayer(ymir::Dir2d::UP);
    break;
  case ui::Controls::Rest.Char:
    // Wait a turn
    Hist.info() << "Resting...";
    break;
  case ui::Controls::Interact.Char:
    if (tryInteract()) {
      break;
    }
    // No interaction, no tick
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
bool Game::tryInteract() {
  auto Player = getPlayer();
  auto PlayerPos = getLvlReg().get<PositionComp>(Player).Pos;

  const auto &CurrentLevel = World->getCurrentLevelOrFail();
  auto InteractableEntities = CurrentLevel.getInteractables(PlayerPos);

  // If there are no interactions we are done
  if (InteractableEntities.empty()) {
    return false;
  }

  // If there is only one action execute it
  if (InteractableEntities.size() == 1) {
    auto &InteractableEntity = InteractableEntities.at(0);
    auto &Interactable = getLvlReg().get<InteractableComp>(InteractableEntity);

    if (Interactable.Actions.size() == 1) {
      auto Player = getPlayer();
      auto &PC = getLvlReg().get<PlayerComp>(Player);
      PC.CurrentInteraction = Interactable.Actions.front();

      // This may switch level so needs to be last thing that is done
      Interactable.Actions.front().Execute(World->getCurrentLevelOrFail(),
                                           Player, getLvlReg());
      return true;
    }
  }

  // Multiple interactions, this show UI to select interaction
  UICtrl.closeInteractUI();
  UICtrl.setInteractUI(getPlayer(), getLvlReg().get<PositionComp>(getPlayer()),
                       World->getCurrentLevelOrFail());

  return true;
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

// FIXME this only returns a single one but there is the possibility to have
// multiple and also cycle the options. We need a proper HUD UI to handle this
Interaction *Game::getAvailableInteraction() {
  auto Player = getPlayer();
  auto PlayerPos = getLvlReg().get<PositionComp>(Player).Pos;

  const auto &CurrentLevel = World->getCurrentLevelOrFail();
  auto InteractableEntities = CurrentLevel.getInteractables(PlayerPos);
  if (InteractableEntities.empty()) {
    return nullptr;
  }

  auto &InteractableEntity = InteractableEntities.at(0);
  auto &Interactable = getLvlReg().get<InteractableComp>(InteractableEntity);
  return &Interactable.Actions.front();
}

void Game::onEntityDiedEvent(const EntityDiedEvent &E) {
  if (E.isPlayerAffected()) {
    GameRunning = false;
  }
}

void Game::onSwitchLevelEvent(const SwitchLevelEvent &E) {
  switchLevel(E.Level, E.ToEntry);
}

void Game::onSwitchGameWorldEvent(const SwitchGameWorldEvent &E) {
  UICtrl.closeAll();
  REC.clear();

  World->switchWorld(Cfg.Seed + WorldSwitchCounter++, E.LevelName, E.SwitchEt);

  // We could update the level here, but we want to draw the initial state.
  handleUpdates(/*IsTick=*/false);
}

void Game::onLootEvent(const LootEvent &E) {
  if (!E.isPlayerAffected() ||
      &World->getCurrentLevelOrFail().Reg != E.Registry) {
    return;
  }
  UICtrl.setLootUI(E.Entity, E.LootedEntity, World->getCurrentLevelOrFail(),
                   E.LootName);
}

void Game::onCraftEvent(const CraftEvent &E) {
  if (!E.isPlayerAffected() ||
      &World->getCurrentLevelOrFail().Reg != E.Registry) {
    return;
  }
  UICtrl.setCraftingUI(E.Entity, World->getCurrentLevelOrFail().Reg, CraftingDb,
                       Crafter);
}

namespace {

ui::Controller::PlayerInfo getUIPlayerInfo(entt::entity Player,
                                           entt::registry &Reg,
                                           Interaction *Interact) {
  const auto &Health = Reg.get<HealthComp>(Player);
  const auto &Mana = Reg.get<ManaComp>(Player);

  ui::Controller::PlayerInfo PI;
  PI.Health = Health.Value;
  PI.MaxHealth = Health.MaxValue;
  PI.Mana = Mana.Value;
  PI.MaxMana = Mana.MaxValue;
  if (Interact) {
    PI.InteractStr = "[e] " + Interact->Msg;
  }

  return PI;
}

std::optional<ui::Controller::TargetInfo> getUITargetInfo(entt::entity Player,
                                                          entt::registry &Reg) {
  const auto *CC = Reg.try_get<CombatAttackComp>(Player);
  if (!CC || CC->Target == entt::null || !Reg.valid(CC->Target)) {
    return {};
  }
  ui::Controller::TargetInfo TI;
  auto *TNC = Reg.try_get<NameComp>(CC->Target);
  auto *THC = Reg.try_get<HealthComp>(CC->Target);
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
  auto PlayerPos = getLvlReg().get<PositionComp>(Player).Pos;
  auto CenterPos = PlayerPos;
  if (auto *TUI = UICtrl.getWindowOfType<ui::TargetUI>()) {
    CenterPos = TUI->getCursorPos();
  }

  auto &CurrentLevel = World->getCurrentLevelOrFail();
  Renderer Render(RenderSize, CurrentLevel, CenterPos);
  Render.renderShadow(/*Darkness=*/30);
  Render.renderFogOfWar(CurrentLevel.getPlayerSeenMap());
  Render.renderAllLineOfSight();
  Render.renderEntities();
  REC.apply(Render);
  REC.clear();

  // Draw map
  Scr << Render.get();

  // Draw UI overlay
  auto PI = getUIPlayerInfo(Player, getLvlReg(), getAvailableInteraction());
  auto TI = getUITargetInfo(Player, getLvlReg());
  UICtrl.draw(World->getCurrentLevelIdx(), PI, TI);

  if (UpdateScreen) {
    handleShowNotifications(false);
    Scr.update();
    Scr.clear();
  }
}

void Game::handleDrawGameOver() {
  info() << "Game Over! Press any key to retry";
  handleShowNotifications(true);
  Scr.update();
  Scr.clear();
  cxxg::utils::sleep(1000000);
  cxxg::utils::getChar(true);
  throw GameOverException();
}

void Game::handleResize(cxxg::types::Size Size) {
  cxxg::Game::handleResize(Size);
  UICtrl.handleResize(Size);
}

} // namespace rogue