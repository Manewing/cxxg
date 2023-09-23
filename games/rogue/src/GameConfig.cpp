#include <fstream>
#include <rapidjson/document.h>
#include <rogue/GameConfig.h>
#include <rogue/JSON.h>

namespace rogue {

GameConfig GameConfig::load(const std::filesystem::path &ConfigFile) {
  GameConfig Config;

  auto [DocStr, Doc] = loadJSON(ConfigFile, nullptr);

  auto ConfigDir = ConfigFile.parent_path();
  const auto LevelConfig = Doc["level_config"].GetString();
  Config.LevelConfig = ConfigDir / LevelConfig;
  const auto ItemDbConfig = Doc["item_db_config"].GetString();
  Config.ItemDbConfig = ConfigDir / ItemDbConfig;

  return Config;
}

} // namespace rogue