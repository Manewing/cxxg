#ifndef ROGUE_CREATURE_DATABASE_H
#define ROGUE_CREATURE_DATABASE_H

#include <filesystem>
#include <map>
#include <rogue/Components/RaceFaction.h>
#include <rogue/Components/Stats.h>
#include <string>
#include <vector>

namespace rogue {

struct CreatureId {
  inline explicit CreatureId(int Id) : Id(Id) {}
  inline operator int() const { return Id; }
  int Id = -1;
};

struct CreatureInfo {
  std::string Name = "";
  std::string Description = "";
  StatPoints Stats = {};
  FactionKind Faction = FactionKind::None;
  RaceKind Race = RaceKind::None;
  CreatureId Id = CreatureId(-1);
};

class CreatureDatabase {
public:
  static CreatureDatabase load(const std::filesystem::path &CreatureDbConfig);

public:
  inline auto empty() const { return CreatureInfos.empty(); }
  inline auto size() const { return CreatureInfos.size(); }

  CreatureId getCreatureId(const std::string &CreatureName) const;
  void addCreature(CreatureInfo CreatureInfo);

  const CreatureInfo &getCreature(CreatureId Id) const;

private:
  /// Map creature name to it's Id
  std::map<std::string, CreatureId> CreatureIdsByName;

  /// Map creature info by Id (==index)
  std::vector<CreatureInfo> CreatureInfos;
};

} // namespace rogue

#endif // #ifndef ROGUE_CREATURE_DATABASE_H