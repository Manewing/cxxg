#ifndef ROGUE_GAME_WORLD_H
#define ROGUE_GAME_WORLD_H

#include <entt/entt.hpp>
#include <filesystem>
#include <memory>
#include <rogue/EventHub.h>
#include <ymir/Types.hpp>

namespace rogue {
class Level;
struct Interaction;
class LevelGenerator;
class LevelDatabase;
} // namespace rogue

namespace rogue {

class GameWorld : public EventHubConnector {
public:
  static std::unique_ptr<GameWorld>
  create(LevelDatabase &LevelDb, LevelGenerator &LvlGen, std::string_view Type);

public:
  virtual ~GameWorld() = default;

  /// Switches to the level with the selected index \p LevelIdx
  /// \param LevelIdx The level index to switch to
  /// \return The level that was switched to
  virtual Level &switchLevel(std::size_t LevelIdx, bool ToEntry) = 0;

  virtual void switchWorld(unsigned Seed, const std::string &LevelName,
                           entt::entity SwitchEt) = 0;

  /// Return the index of the currently active level
  virtual std::size_t getCurrentLevelIdx() const = 0;

  /// Return the currently active level
  virtual Level *getCurrentLevel() = 0;
  virtual const Level *getCurrentLevel() const = 0;
  virtual Level &getCurrentLevelOrFail() = 0;
  virtual const Level &getCurrentLevelOrFail() const = 0;
};

class MultiLevelDungeon : public GameWorld {
public:
  static constexpr const char *Type = "multi_level_dungeon";

public:
  explicit MultiLevelDungeon(LevelGenerator &LvlGen);

  /// Switches to the level with the selected index \p LevelIdx
  /// \param LevelIdx The level index to switch to
  /// \return The level that was switched to
  Level &switchLevel(std::size_t LevelIdx, bool ToEntry) override;

  // FIXME
  void switchWorld(unsigned Seed, const std::string &LevelName,
                   entt::entity SwitchEt) override;

  /// Return the index of the currently active level
  std::size_t getCurrentLevelIdx() const override;

  /// Return the currently active level
  Level *getCurrentLevel() override;
  const Level *getCurrentLevel() const override;
  Level &getCurrentLevelOrFail() override;
  const Level &getCurrentLevelOrFail() const override;

private:
  LevelGenerator &LevelGen;
  std::size_t CurrentLevelIdx = 0;
  std::vector<std::shared_ptr<Level>> Levels;
};

/// Arena to allow watching NPC vs NPC fights
class ArenaView : public GameWorld {
public:
  // TODO
};

/// Single map level that allows to switch between multi-level dungeons
class DungeonSweeper : public GameWorld {
public:
  static constexpr const char *Type = "dungeon_sweeper";

public:
  explicit DungeonSweeper(LevelDatabase &LevelDb, LevelGenerator &LvlGen);

  /// Switches to the level with the selected index \p LevelIdx
  /// \param LevelIdx The level index to switch to
  /// \return The level that was switched to
  // FIXME
  // LevelIdx == 0 -> top level selection
  // LevelIdx > 1 -> individual levels
  Level &switchLevel(std::size_t LevelIdx, bool ToEntry) override;

  // FIXME
  void switchWorld(unsigned Seed, const std::string &LevelName,
                   entt::entity SwitchEt) override;

  // FIXME
  /// Return the index of the currently active level
  std::size_t getCurrentLevelIdx() const override;

  /// Return the currently active level
  Level *getCurrentLevel() override;
  const Level *getCurrentLevel() const override;
  Level &getCurrentLevelOrFail() override;
  const Level &getCurrentLevelOrFail() const override;

protected:
  void switchWorld(unsigned Seed, std::string_view Type,
                   std::filesystem::path Config, entt::entity SwitchEt);

private:
  LevelDatabase &LevelDb;
  LevelGenerator &LevelGen;
  std::shared_ptr<Level> Lvl;
  std::shared_ptr<LevelGenerator> CurrSubLvlGen = nullptr;
  std::unique_ptr<GameWorld> CurrSubWorld = nullptr;
  std::size_t CurrMaxLevel = 0;
  entt::entity CurrSwitchEntity = entt::null;
  int SwitchedWorldCount = 0;
};

/// A chunk based procedurally generated infinite world
class OverWorld : public GameWorld {
public:
  // TODO
};

} // namespace rogue

#endif // #ifndef ROGUE_GAME_WORLD_H