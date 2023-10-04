#ifndef ROGUE_GAME_CONFIG_H
#define ROGUE_GAME_CONFIG_H

#include <filesystem>
#include <vector>

namespace rogue {

struct LevelConfig {
  int LevelEndIdx = 0;
  std::filesystem::path Config;
};

struct GameConfig {
  std::filesystem::path CreatureDbConfig;
  std::filesystem::path ItemDbConfig;
  std::vector<LevelConfig> Levels;

  static GameConfig load(const std::filesystem::path &ConfigFile);

  const LevelConfig &getLevelConfig(int LevelIdx) const;
};

} // namespace rogue

#endif // #ifndef ROGUE_GAME_CONFIG_H