#ifndef ROGUE_GAME_H
#define ROGUE_GAME_H

#include <cxxg/Game.h>
#include <memory>
#include <rogue/Context.h>
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

class Game : public cxxg::Game {
public:
  Game(cxxg::Screen &Scr);
  virtual ~Game() = default;

  void initialize(bool BufferedInput = false, unsigned TickDelayUs = 0) final;

  void switchLevel(int Level);

  bool handleInput(int Char) final;

  void handleDraw() final;

  void movePlayer(ymir::Dir2d Dir);
  void tryInteract();

private:
  entt::registry &getLvlReg();

  entt::entity getPlayerOrNull() const;
  entt::entity getPlayer() const;

  /// Update level and handle entity updates
  bool handleUpdates(bool IsTick);

  void updateCurrentInteraction();

public: // FIXME
  EventHub EvHub;
  History Hist;
  EventHistoryWriter EHW;
  ItemDatabase ItemDb;
  GameContext Ctx;
  LevelGenerator LevelGen;
  int CurrentLevelIdx = 0;
  std::shared_ptr<Level> CurrentLevel;
  std::vector<std::shared_ptr<Level>> Levels;

  ui::Controller UICtrl;

  long unsigned GameTicks = 0;
};

} // namespace rogue

#endif // #ifndef ROGUE_GAME_H