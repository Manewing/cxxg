#include <fstream>
#include <rapidjson/document.h>
#include <rogue/GameConfig.h>
#include <rogue/JSON.h>

namespace rogue {

GameConfig GameConfig::load(const std::filesystem::path &ConfigFile) {
  GameConfig Config;

  const auto SchemaFile =
      ConfigFile.parent_path() / "schemas" / "game_config_schema.json";
  auto [DocStr, Doc] = loadJSON(ConfigFile, &SchemaFile);

  auto ConfigDir = ConfigFile.parent_path();

  const auto ItemDbConfig = Doc["item_db_config"].GetString();
  Config.ItemDbConfig = ConfigDir / ItemDbConfig;

  auto EntityDbConfig = Doc["entity_db_config"].GetString();
  Config.EntityDbConfig = ConfigDir / EntityDbConfig;

  const auto LevelDbConfig = Doc["level_db_config"].GetString();
  Config.LevelDbConfig = ConfigDir / LevelDbConfig;

  const auto CraftingDbConfig = Doc["crafting_db_config"].GetString();
  Config.CraftingDbConfig = ConfigDir / CraftingDbConfig;

  Config.InitialGameWorld = Doc["initial_game_world"].GetString();

  auto InitialLevelConfig = Doc["initial_level_config"].GetString();
  Config.InitialLevelConfig = ConfigDir / InitialLevelConfig;

  for (const auto &Item : Doc["initial_items"].GetArray()) {
    PlayerInitialItemConfig ItemCfg;
    ItemCfg.Name = Item["name"].GetString();
    ItemCfg.Count = Item["count"].GetUint();
    Config.InitialItems.push_back(ItemCfg);
  }

  return Config;
}

std::ostream &operator<<(std::ostream &Out, const GameConfig &Cfg) {
  Out << "GameConfig:\n"
      << "  Seed: " << Cfg.Seed << "\n"
      << "  ItemDbConfig: " << Cfg.ItemDbConfig << "\n"
      << "  EntityDbConfig: " << Cfg.EntityDbConfig << "\n"
      << "  LevelDbConfig: " << Cfg.LevelDbConfig << "\n"
      << "  Level: " << Cfg.InitialLevelConfig << "\n"
      << "  GameWorld: " << Cfg.InitialGameWorld << "\n"
      << "  InitialItems:\n";
  for (const auto &Item : Cfg.InitialItems) {
    Out << "    -> " << Item.Name << " x" << Item.Count << "\n";
  }
  return Out;
}

} // namespace rogue