#ifndef ROGUE_GAME_CONFIG_H
#define ROGUE_GAME_CONFIG_H

#include <filesystem>
#include <vector>

namespace rogue {

struct LevelRangeConfig {
  int LevelEndIdx = 0;
  std::filesystem::path Config;
};

struct GameConfig {
  std::filesystem::path CreatureDbConfig;
  std::filesystem::path ItemDbConfig;
  std::vector<LevelRangeConfig> Levels;

  static GameConfig load(const std::filesystem::path &ConfigFile);

  const LevelRangeConfig &getLevelRangeConfig(int LevelIdx) const;
};

} // namespace rogue

#endif // #ifndef ROGUE_GAME_CONFIG_H