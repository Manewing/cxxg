#include <algorithm>
#include <rogue/JSON.h>
#include <rogue/LevelDatabase.h>

namespace rogue {

LevelInstance::LevelInstance(std::string WorldType,
                             std::filesystem::path LevelConfig)
    : Info({WorldType, LevelConfig}) {}

const LevelInstance::LevelInfo &LevelInstance::getLevelInfo() const {
  return Info;
}

std::size_t LevelTable::getSlotForRoll(int Roll,
                                       const std::vector<LevelSlot> &Slots) {
  for (std::size_t Idx = 0; Idx < Slots.size(); ++Idx) {
    Roll -= Slots.at(Idx).Weight;
    if (Roll < 0) {
      return Idx;
    }
  }
  throw std::runtime_error("LevelTable::getSlotForRoll() failed");
}

std::size_t LevelTable::rollForSlot(const std::vector<LevelSlot> &Slots) {
  int TotalWeight = 0;
  for (const auto &Slot : Slots) {
    TotalWeight += Slot.Weight;
  }
  int Roll = std::rand() % TotalWeight;
  return getSlotForRoll(Roll, Slots);
}

LevelTable::LevelTable() { reset({}); }

LevelTable::LevelTable(const std::vector<LevelSlot> &Sls) { reset(Sls); }

void LevelTable::reset(const std::vector<LevelSlot> &Sls) {
  Slots = Sls;
  std::stable_sort(
      Slots.begin(), Slots.end(),
      [](const auto &A, const auto &B) { return A.Weight < B.Weight; });
}

const std::vector<LevelTable::LevelSlot> &LevelTable::getSlots() const {
  return Slots;
}

const LevelContainer::LevelInfo &LevelTable::getLevelInfo() const {
  if (Slots.empty()) {
    throw std::runtime_error("LevelTable::getLevelInfo() failed");
  }
  std::size_t Idx = rollForSlot(Slots);
  return Slots.at(Idx).LC->getLevelInfo();
}

LevelDatabase LevelDatabase::load(const std::filesystem::path &LevelDbConfig) {
  LevelDatabase DB;

  const auto BasePath = LevelDbConfig.parent_path();

  const auto SchemaPath = BasePath / "schemas" / "level_db_schema.json";
  // FIXME avoid requiring schema for tests
  const auto *SchemaPathPtr = &SchemaPath;
  if (!std::filesystem::exists(SchemaPath)) {
    SchemaPathPtr = nullptr;
  }
  auto [DocStr, Doc] = loadJSON(LevelDbConfig, SchemaPathPtr);

  // Create level tables
  const auto &LevelTablesJson = Doc["level_tables"].GetObject();
  for (const auto &[K, V] : LevelTablesJson) {
    const auto Name = std::string(K.GetString());
    DB.addLevelTable(Name, std::make_shared<LevelTable>());
  }

  // Fill all tables
  for (const auto &[K, V] : LevelTablesJson) {
    // Fetch the table to fill
    const auto Name = std::string(K.GetString());
    const auto &LT = DB.getLevelTable(Name);

    // Fill the table from the slots
    const auto &SlotsJson = V.GetObject()["slots"].GetArray();
    std::vector<LevelTable::LevelSlot> Slots;
    for (const auto &SlotJson : SlotsJson) {
      const auto &SlotJsonObj = SlotJson.GetObject();
      const auto Type = std::string(SlotJsonObj["type"].GetString());
      const auto Weight = SlotJsonObj["weight"].GetInt();
      if (Type == "table") {
        const auto &TableName = SlotJsonObj["ref"].GetString();
        Slots.push_back({DB.getLevelTable(TableName), Weight});
        continue;
      }
      if (Type != "level") {
        throw std::runtime_error("LevelDatabase: unknown slot type " + Type);
      }
      const auto GameWorld = std::string(SlotJsonObj["game_world"].GetString());
      const auto LevelConfig =
          BasePath / SlotJsonObj["level_config"].GetString();

      auto LevelInst = std::make_shared<LevelInstance>(GameWorld, LevelConfig);
      Slots.push_back({LevelInst, Weight});
    }

    LT->reset(Slots);
  }

  return DB;
}

const std::shared_ptr<LevelTable> &
LevelDatabase::getLevelTable(const std::string &LevelName) const {
  auto It = LevelsByName.find(LevelName);
  if (It == LevelsByName.end()) {
    throw std::out_of_range("LevelDatabase: no level with name " + LevelName);
  }
  return It->second;
}

const LevelContainer::LevelInfo &
LevelDatabase::getLevelInfo(const std::string &LevelName) const {
  auto It = LevelsByName.find(LevelName);
  if (It == LevelsByName.end()) {
    throw std::out_of_range("LevelDatabase: no level with name " + LevelName);
  }
  return It->second->getLevelInfo();
}

void LevelDatabase::addLevelTable(const std::string &LevelName,
                                  std::shared_ptr<LevelTable> LC) {
  auto It = LevelsByName.find(LevelName);
  if (It != LevelsByName.end()) {
    throw std::out_of_range("LevelDatabase: level with name " + LevelName +
                            " already exists");
  }
  LevelsByName[LevelName] = LC;
}

} // namespace rogue