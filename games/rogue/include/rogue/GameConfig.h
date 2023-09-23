#ifndef ROGUE_GAME_CONFIG_H
#define ROGUE_GAME_CONFIG_H

#include <filesystem>

namespace rogue {

struct GameConfig {
  std::filesystem::path LevelConfig;
  std::filesystem::path ItemDbConfig;

  static GameConfig load(const std::filesystem::path &ConfigFile);
};

} // namespace rogue

#endif // #ifndef ROGUE_GAME_CONFIG_H