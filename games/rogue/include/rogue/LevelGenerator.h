#ifndef ROGUE_LEVEL_GENERATOR_H
#define ROGUE_LEVEL_GENERATOR_H

#include <filesystem>
#include <map>
#include <memory>
#include <rogue/Level.h>
#include <rogue/Tile.h>
#include <vector>
#include <ymir/Types.hpp>

namespace rogue {
struct GameContext;
}

namespace rogue {

class LevelGenerator {
public:
  struct CharInfo {
    Tile T;
    std::string Layer;
  };
  explicit LevelGenerator(GameContext *Ctx = nullptr);

  std::shared_ptr<Level> generateLevel(unsigned Seed, int LevelId);
  std::shared_ptr<Level> loadLevel(const std::filesystem::path &LevelFile,
                                   const std::vector<std::string> &Layers,
                                   const std::map<char, CharInfo> &CharInfoMap,
                                   int LevelId);

protected:
  void spawnEntities(Level &L);
  void spawnEntity(Level &L, ymir::Point2d<int> Pos, Tile T);

protected:
  GameContext *Ctx = nullptr;
};

} // namespace rogue

#endif // #ifndef ROGUE_LEVEL_GENERATOR_H