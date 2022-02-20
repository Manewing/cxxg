#ifndef ROGUE_GAME_H
#define ROGUE_GAME_H

#include "ItemDatabase.h"
#include "Level.h"
#include "LevelGenerator.h"
#include "UIController.h"
#include <cxxg/Game.h>
#include <memory>
#include <ymir/LayeredMap.hpp>
#include <ymir/Map.hpp>
#include <ymir/Types.hpp>

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

public: // FIXME
  ItemDatabase ItemDb;
  LevelGenerator LevelGen;
  std::unique_ptr<PlayerEntity> Player;
  int CurrentLevelIdx = 0;
  std::shared_ptr<Level> CurrentLevel;
  std::vector<std::shared_ptr<Level>> Levels;

  UIController UICtrl;
};

#endif // #ifndef ROGUE_GAME_H