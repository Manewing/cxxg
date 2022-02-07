#ifndef ROGUE_LEVEL_GENERATOR_H
#define ROGUE_LEVEL_GENERATOR_H

#include "Level.h"
#include <memory>

class LevelGenerator {
public:
  std::shared_ptr<Level> generateLevel(unsigned Seed);
};

#endif // #ifndef ROGUE_LEVEL_GENERATOR_H