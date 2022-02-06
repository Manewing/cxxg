#ifndef ROGUE_GAME_H
#define ROGUE_GAME_H

#include <cxxg/Game.h>
#include <ymir/Types.hpp>
#include <ymir/Map.hpp>

class Game : public cxxg::Game {
public:
  Game(cxxg::Screen &Scr);
  virtual ~Game() = default;

  void initialize(bool BufferedInput = false, unsigned TickDelayUs = 0) final;
  void handleInput(int Char) final;
  void handleDraw() final;

  void renderShadow(unsigned char Darkness);
  void renderLineOfSight(ymir::Point2d<int> AtPos, unsigned int Range);

  void movePlayer(ymir::Dir2d Dir);

  ymir::Point2d<int> PlayerPos = {0, 0};
  ymir::Map<cxxg::types::ColoredChar> LevelMap;
  ymir::Map<cxxg::types::ColoredChar> VisibleMap;
};

#endif // #ifndef ROGUE_GAME_H