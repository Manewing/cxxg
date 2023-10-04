#ifndef ROGUE_GAME_H
#define ROGUE_GAME_H

#include <cxxg/Game.h>
#include <memory>
#include <rogue/Context.h>
#include <rogue/CreatureDatabase.h>
#include <rogue/EventHub.h>
#include <rogue/History.h>
#include <rogue/ItemDatabase.h>
#include <rogue/Level.h>
#include <rogue/LevelGenerator.h>
#include <rogue/UI/Controller.h>
#include <ymir/LayeredMap.hpp>
#include <ymir/Map.hpp>
#include <ymir/Types.hpp>

namespace rogue {
struct Interaction;
struct GameConfig;
struct EntityDiedEvent;
struct SwitchLevelEvent;
struct LootEvent;
struct EntityAttackEvent;
class Renderer;
} // namespace rogue

namespace rogue {

class RenderEventCollector : public EventHubConnector {
public:
  void setEventHub(EventHub *EH) override;
  void onEntityAttackEvent(const EntityAttackEvent &E);
  void apply(Renderer &R);
  void clear();
  bool hasEvents() const;

private:
  std::vector<std::function<void(Renderer &)>> RenderFns;
};

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
  void tryInteract();

private:
  entt::registry &getLvlReg();

  entt::entity getPlayerOrNull() const;
  entt::entity getPlayer() const;

  Interaction *getAvailableInteraction();

  void onEntityDiedEvent(const EntityDiedEvent &E);
  void onSwitchLevelEvent(const SwitchLevelEvent &E);
  void onLootEvent(const LootEvent &E);

  void handleDrawLevel(bool UpdateScreen);
  void handleDrawGameOver();

private:
  const GameConfig &Cfg;
  EventHub EvHub;
  History Hist;
  EventHistoryWriter EHW;
  ItemDatabase ItemDb;
  CreatureDatabase CreatureDb;
  GameContext Ctx;
  LevelGenerator LevelGen;
  int CurrentLevelIdx = 0;
  std::shared_ptr<Level> CurrentLevel;
  RenderEventCollector REC;
  std::vector<std::shared_ptr<Level>> Levels;
  ui::Controller UICtrl;
  long unsigned GameTicks = 0;
};

} // namespace rogue

#endif // #ifndef ROGUE_GAME_H