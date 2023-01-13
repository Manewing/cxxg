#ifndef ROGUE_COMPONENTS_LEVEL_H
#define ROGUE_COMPONENTS_LEVEL_H

namespace rogue {

struct LevelStartComp {
  int NextLevelId = -1;
};

struct LevelEndComp {
  int NextLevelId = -1;
};

}

#endif // #ifndef ROGUE_COMPONENTS_LEVEL_H