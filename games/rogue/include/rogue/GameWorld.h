#ifndef ROGUE_GAME_WORLD_H
#define ROGUE_GAME_WORLD_H

#include <entt/entt.hpp>
#include <memory>
#include <rogue/EventHub.h>
#include <ymir/Types.hpp>

namespace rogue {
class Level;
struct Interaction;
class LevelGenerator;
} // namespace rogue

namespace rogue {

class GameWorld : public EventHubConnector {
public:
  virtual ~GameWorld() = default;

  /// Return the currently active level
  virtual Level *getCurrentLevel() = 0;
  virtual const Level *getCurrentLevel() const = 0;
  virtual Level &getCurrentLevelOrFail() = 0;
  virtual const Level &getCurrentLevelOrFail() const = 0;

protected:
};

class MultiLevelDungeon : public GameWorld {
public:
  explicit MultiLevelDungeon(LevelGenerator &LvlGen);

  /// Switches to the level with the selected index \p LevelIdx
  /// \param LevelIdx The level index to switch to
  void switchLevel(std::size_t LevelIdx);

  /// Return the currently active level
  Level *getCurrentLevel() override;
  const Level *getCurrentLevel() const override;
  Level &getCurrentLevelOrFail() override;
  const Level &getCurrentLevelOrFail() const override;

  void moveEntity(entt::entity Entity, ymir::Point2d<int> ToPos);

  void tryInteract(entt::entity Entity, ymir::Point2d<int> AtPos);
  Interaction *getInteraction(ymir::Point2d<int> AtPos);

private:
  LevelGenerator &LevelGen;
  std::size_t CurrentLevelIdx = 0;
  // RenderEventCollector REC;
  std::vector<std::shared_ptr<Level>> Levels;
};

/// Arena to allow watching NPC vs NPC fights
class ArenaView : public GameWorld {
public:
};

/// Single map level that allows to switch between multi-level dungeons
//
// ?????
// ?xxy?
// ?x@y?
// ?zzy?
// ?????
//
class DungeonSweeper : public GameWorld {
public:
  /// Return the currently active level
  Level *getCurrentLevel() override;
  const Level *getCurrentLevel() const override;
  Level &getCurrentLevelOrFail() override;
  const Level &getCurrentLevelOrFail() const override;

private:
  std::shared_ptr<Level> Lvl;
};

/// A chunk based procedurally generated infinite world
class OverWorld : public GameWorld {};

} // namespace rogue

#endif // #ifndef ROGUE_GAME_WORLD_H