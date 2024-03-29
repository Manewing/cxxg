#ifndef ROGUE_GAME_CONFIG_H
#define ROGUE_GAME_CONFIG_H

#include <filesystem>
#include <iosfwd>
#include <vector>

namespace rogue {

struct PlayerInitialItemConfig {
  std::string Name;
  unsigned Count = 1;
};

struct GameConfig {
  unsigned Seed = 0;
  std::filesystem::path ItemDbConfig;
  std::filesystem::path EntityDbConfig;
  std::filesystem::path LevelDbConfig;
  std::filesystem::path CraftingDbConfig;
  std::string InitialGameWorld;
  std::filesystem::path InitialLevelConfig;
  std::vector<PlayerInitialItemConfig> InitialItems;

  static GameConfig load(const std::filesystem::path &ConfigFile);
};

std::ostream &operator<<(std::ostream &Out, const GameConfig &Cfg);

} // namespace rogue

#endif // #ifndef ROGUE_GAME_CONFIG_H