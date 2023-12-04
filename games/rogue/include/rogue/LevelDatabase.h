#ifndef ROGUE_LEVEL_DATABASE_H
#define ROGUE_LEVEL_DATABASE_H

#include <filesystem>
#include <map>
#include <memory>
#include <vector>

namespace rogue {

/// Base container for holding level information
class LevelContainer {
public:
  struct LevelInfo {
    std::string WorldType;
    std::filesystem::path LevelConfig;
  };

public:
  virtual ~LevelContainer() = default;
  virtual const LevelInfo &getLevelInfo() const = 0;
};

/// Container holding information on a single level
class LevelInstance : public LevelContainer {
public:
  LevelInstance() = delete;
  LevelInstance(std::string WorldType, std::filesystem::path LevelConfig);
  const LevelInfo &getLevelInfo() const final;

private:
  LevelInfo Info;
};

/// Container holding information on a level table, allowing random selection of
/// levels
class LevelTable : public LevelContainer {
public:
  struct LevelSlot {
    /// Level container for the slot
    std::shared_ptr<LevelContainer> LC = nullptr;

    /// Weight for the slot
    int Weight = 0;
  };

public:
  static std::size_t getSlotForRoll(int Roll,
                                    const std::vector<LevelSlot> &Slots);
  static std::size_t rollForSlot(const std::vector<LevelSlot> &Slots);

public:
  LevelTable();
  explicit LevelTable(const std::vector<LevelSlot> &Slots);
  void reset(const std::vector<LevelSlot> &Slots);

  /// Return all slots in the table
  const std::vector<LevelSlot> &getSlots() const;

  /// Roll for a level and return the information
  const LevelInfo &getLevelInfo() const final;

private:
  std::vector<LevelSlot> Slots;
};

/// Database containing configuration for all levels
class LevelDatabase {
public:
  static LevelDatabase load(const std::filesystem::path &LevelDbConfig);

public:
  const std::shared_ptr<LevelTable> &
  getLevelTable(const std::string &LevelName) const;

  const LevelContainer::LevelInfo &
  getLevelInfo(const std::string &LevelName) const;

  void addLevelTable(const std::string &LevelName,
                     std::shared_ptr<LevelTable> LC);

private:
  std::map<std::string, std::shared_ptr<LevelTable>> LevelsByName;
};

} // namespace rogue

#endif // ROGUE_LEVEL_DATABASE_H