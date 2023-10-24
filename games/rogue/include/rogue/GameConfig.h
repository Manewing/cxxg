#ifndef ROGUE_GAME_CONFIG_H
#define ROGUE_GAME_CONFIG_H

#include <filesystem>
#include <vector>
#include <iosfwd>

namespace rogue {

struct LevelRangeConfig {
  int LevelEndIdx = 0;
  std::filesystem::path Config;
};

struct GameConfig {
  unsigned Seed = 0;
  std::filesystem::path CreatureDbConfig;
  std::filesystem::path ItemDbConfig;
  std::vector<LevelRangeConfig> Levels;

  static GameConfig load(const std::filesystem::path &ConfigFile);

  const LevelRangeConfig &getLevelRangeConfig(int LevelIdx) const;
};

std::ostream &operator<<(std::ostream &Out, const GameConfig &Cfg);

} // namespace rogue

#endif // #ifndef ROGUE_GAME_CONFIG_H