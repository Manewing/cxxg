#ifndef ROGUE_LEVEL_GENERATOR_H
#define ROGUE_LEVEL_GENERATOR_H

#include "Level.h"
#include "Tile.h"
#include <memory>
#include <ymir/Types.hpp>

class LevelGenerator {
public:
  std::shared_ptr<Level> generateLevel(unsigned Seed);

protected:
  void spawnEnemies(Level &L);
  void spawnEnemy(Level &L, ymir::Point2d<int> Pos, Tile T);
};

#endif // #ifndef ROGUE_LEVEL_GENERATOR_H