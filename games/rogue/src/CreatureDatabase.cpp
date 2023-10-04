#include <rogue/CreatureDatabase.h>
#include <rogue/JSON.h>

namespace rogue {

static StatPoints parseStatPoints(const rapidjson::Value &V) {
  StatPoints P;
  P.Str = V["str"].GetInt();
  P.Dex = V["dex"].GetInt();
  P.Int = V["int"].GetInt();
  P.Vit = V["vit"].GetInt();
  return P;
}

static CreatureInfo parseCreatueInfo(const rapidjson::Value &V) {
  const auto Name = std::string(V["name"].GetString());
  const auto Description = std::string(V["description"].GetString());
  const auto Stats = parseStatPoints(V["stats"]);
  const auto Faction = getFaction(V["faction"].GetString());
  const auto Race = getRace(V["race"].GetString());
  return CreatureInfo{Name, Description, Stats, Faction, Race};
}

CreatureDatabase
CreatureDatabase::load(const std::filesystem::path &CreatureDbConfig) {
  CreatureDatabase Db;

  const auto SchemaPath =
      CreatureDbConfig.parent_path() / "creature_db_schema.json";
  auto [DocStr, Doc] = loadJSON(CreatureDbConfig, &SchemaPath);

  auto Creatures = Doc["creatures"].GetArray();
  for (const auto &CreatureJson : Creatures) {
    Db.addCreature(parseCreatueInfo(CreatureJson));
  }

  return Db;
}

CreatureId
CreatureDatabase::getCreatureId(const std::string &CreatureName) const {
  return CreatureIdsByName.at(CreatureName);
}

void CreatureDatabase::addCreature(CreatureInfo CreatureInfo) {
  auto It = CreatureIdsByName.find(CreatureInfo.Name);
  if (It != CreatureIdsByName.end()) {
    throw std::out_of_range("Duplicate creature name: " + CreatureInfo.Name);
  }
  CreatureInfo.Id = CreatureId(CreatureInfos.size());
  CreatureIdsByName.emplace(CreatureInfo.Name, CreatureInfo.Id);
  CreatureInfos.emplace_back(std::move(CreatureInfo));
}

const CreatureInfo &CreatureDatabase::getCreature(CreatureId Id) const {
  return CreatureInfos.at(Id);
}

} // namespace rogue
