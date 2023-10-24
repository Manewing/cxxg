#include <fstream>
#include <rapidjson/document.h>
#include <rogue/GameConfig.h>
#include <rogue/JSON.h>

namespace rogue {

GameConfig GameConfig::load(const std::filesystem::path &ConfigFile) {
  GameConfig Config;

  const auto SchemaFile = ConfigFile.parent_path() / "game_config_schema.json";
  auto [DocStr, Doc] = loadJSON(ConfigFile, &SchemaFile);

  auto ConfigDir = ConfigFile.parent_path();
  const auto ItemDbConfig = Doc["item_db_config"].GetString();
  Config.ItemDbConfig = ConfigDir / ItemDbConfig;
  auto CreatureDbConfig = Doc["creature_db_config"].GetString();
  Config.CreatureDbConfig = ConfigDir / CreatureDbConfig;

  for (const auto &Level : Doc["levels"].GetArray()) {
    LevelRangeConfig LevelCfg;
    LevelCfg.LevelEndIdx = Level["end"].GetInt();
    const auto LevelConfig = Level["config"].GetString();
    LevelCfg.Config = ConfigDir / LevelConfig;
    Config.Levels.push_back(LevelCfg);
  }

  for (const auto &Item : Doc["initial_items"].GetArray()) {
    PlayerInitialItemConfig ItemCfg;
    ItemCfg.Name = Item["name"].GetString();
    ItemCfg.Count = Item["count"].GetUint();
    Config.InitialItems.push_back(ItemCfg);
  }

  return Config;
}

const LevelRangeConfig &GameConfig::getLevelRangeConfig(int LevelIdx) const {
  for (const auto &Level : Levels) {
    if (Level.LevelEndIdx >= LevelIdx) {
      return Level;
    }
  }
  throw std::runtime_error("No level config for level " +
                           std::to_string(LevelIdx));
}

std::ostream &operator<<(std::ostream &Out, const GameConfig &Cfg) {
  Out << "GameConfig:\n";
  Out << "  Seed: " << Cfg.Seed << "\n";
  Out << "  ItemDbConfig: " << Cfg.ItemDbConfig << "\n";
  Out << "  CreatureDbConfig: " << Cfg.CreatureDbConfig << "\n";
  Out << "  Levels:\n";
  for (const auto &Level : Cfg.Levels) {
    Out << "    LevelEndIdx: " << Level.LevelEndIdx << "\n";
    Out << "    Config: " << Level.Config << "\n";
  }
  Out << "  InitialItems:\n";
  for (const auto &Item : Cfg.InitialItems) {
    Out << "  InitialItem: " << Item.Name << " x" << Item.Count << "\n";
  }
  return Out;
}

} // namespace rogue