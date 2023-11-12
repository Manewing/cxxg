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
  Out << "GameConfig:\n";
  Out << "  Seed: " << Cfg.Seed << "\n";
  Out << "  ItemDbConfig: " << Cfg.ItemDbConfig << "\n";
  Out << "  CreatureDbConfig: " << Cfg.CreatureDbConfig << "\n";
  Out << "  Level: " << Cfg.InitialLevelConfig << "\n";
  Out << "  GameWorld: " << Cfg.InitialGameWorld << "\n";
  Out << "  InitialItems:\n";
  for (const auto &Item : Cfg.InitialItems) {
    Out << "    -> " << Item.Name << " x" << Item.Count << "\n";
  }
  return Out;
}

} // namespace rogue