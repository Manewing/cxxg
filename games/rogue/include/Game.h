#ifndef ROGUE_GAME_H
#define ROGUE_GAME_H

#include <cxxg/Game.h>
#include <ymir/Types.hpp>
#include <ymir/Map.hpp>
#include <ymir/LayeredMap.hpp>
#include "LevelGenerator.h"
#include "Level.h"
#include <memory>

class Game : public cxxg::Game {
public:
  Game(cxxg::Screen &Scr);
  virtual ~Game() = default;

  void initialize(bool BufferedInput = false, unsigned TickDelayUs = 0) final;

  void switchLevel(int Level);

  void handleInput(int Char) final;
  void handleDraw() final;

  void movePlayer(ymir::Dir2d Dir);
  void tryInteract();

public: // FIXME
  ymir::Point2d<int> PlayerPos = {0, 0};
  struct Interaction {
    std::string Msg;
    std::function<void()> Finalize = [](){};
  };
  std::optional<Interaction> CurrentInteraction;


  LevelGenerator LevelGen;
  int CurrentLevelIdx = 0;
  std::shared_ptr<Level> CurrentLevel;
  std::vector<std::shared_ptr<Level>> Levels;
};

#endif // #ifndef ROGUE_GAME_H