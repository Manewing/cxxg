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

struct LevelConfig {
public:
  struct Creature {
    std::string Name;
  };

  struct Chest {
    std::string LootTableName;
  };

  struct GeneratedMap {
    std::filesystem::path Config;
  };

  struct DesignedMap {
    struct CharInfo {
      Tile T;
      std::string Layer;
    };

    std::filesystem::path MapFile;
    CharInfo DefaultChar;
    std::map<char, CharInfo> CharInfoMap;
  };

  using MapConfig = std::variant<GeneratedMap, DesignedMap>;

public:
  MapConfig Map;
  std::map<char, Creature> Creatures;
  std::map<char, Chest> Chests;
};

class LevelGenerator {
public:
  explicit LevelGenerator(GameContext *Ctx = nullptr);

  std::shared_ptr<Level>
  generateLevel(unsigned Seed, int LevelId,
                const std::filesystem::path &CfgFile);

  std::shared_ptr<Level>
  generateLevel(unsigned Seed, int LevelId, const LevelConfig &Cfg);

protected:
  void spawnEntities(const LevelConfig &Cfg, Level &L);
  void spawnEntity(const LevelConfig &Cfg, Level &L, ymir::Point2d<int> Pos,
                   Tile T);

protected:
  GameContext *Ctx = nullptr;
};

} // namespace rogue

#endif // #ifndef ROGUE_LEVEL_GENERATOR_H