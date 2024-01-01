#ifndef ROGUE_GAME_H
#define ROGUE_GAME_H

#include <cxxg/Game.h>
#include <memory>
#include <rogue/Context.h>
#include <rogue/CraftingHandler.h>
#include <rogue/EntityDatabase.h>
#include <rogue/EventHub.h>
#include <rogue/GameWorld.h>
#include <rogue/History.h>
#include <rogue/ItemDatabase.h>
#include <rogue/Level.h>
#include <rogue/LevelDatabase.h>
#include <rogue/LevelGenerator.h>
#include <rogue/RenderEventCollector.h>
#include <rogue/UI/Controller.h>
#include <ymir/LayeredMap.hpp>
#include <ymir/Map.hpp>
#include <ymir/Types.hpp>

namespace rogue {
struct Interaction;
struct GameConfig;
struct EntityDiedEvent;
struct SwitchLevelEvent;
struct SwitchGameWorldEvent;
struct LootEvent;
struct CraftEvent;
} // namespace rogue

namespace rogue {

class GameOverException {};

class Game : public cxxg::Game {
public:
  Game(cxxg::Screen &Scr, const GameConfig &Cfg);
  virtual ~Game() = default;

  void initialize(bool BufferedInput = false, unsigned TickDelayUs = 0) final;

  /// Switches to the level with the selected index \p Level
  /// \param Level The level index to switch to
  /// \param ToEntry If true, the player will be moved to the entry else exit
  void switchLevel(int Level, bool ToEntry);

  bool handleInput(int Char) final;

  /// Update level and handle entity updates
  bool handleUpdates(bool IsTick);

  void handleDraw() final;

  void movePlayer(ymir::Dir2d Dir);
  bool tryInteract();

private:
  entt::registry &getLvlReg();

  entt::entity getPlayerOrNull() const;
  entt::entity getPlayer() const;

  Interaction *getAvailableInteraction();

  void onEntityDiedEvent(const EntityDiedEvent &E);
  void onSwitchLevelEvent(const SwitchLevelEvent &E);
  void onSwitchGameWorldEvent(const SwitchGameWorldEvent &E);
  void onLootEvent(const LootEvent &E);
  void onCraftEvent(const CraftEvent &E);

  void handleDrawLevel(bool UpdateScreen);
  void handleDrawGameOver();

  void handleResize(cxxg::types::Size Size) final;

private:
  const GameConfig &Cfg;
  EventHub EvHub;
  History Hist;
  EventHistoryWriter EHW;

  ItemDatabase ItemDb;
  EntityDatabase EntityDb;
  LevelDatabase LevelDb;
  CraftingDatabase CraftingDb;
  CraftingHandler Crafter;

  GameContext Ctx;

  std::shared_ptr<LevelGenerator> LvlGen;
  std::unique_ptr<GameWorld> World;

  RenderEventCollector REC;
  ui::Controller UICtrl;
  long unsigned GameTicks = 0;
};

} // namespace rogue

#endif // #ifndef ROGUE_GAME_H