#ifndef ROGUE_LEVEL_H
#define ROGUE_LEVEL_H

#include <entt/entt.hpp>
#include <memory>
#include <rogue/EventHub.h>
#include <rogue/Tile.h>
#include <vector>
#include <ymir/LayeredMap.hpp>
#include <ymir/Types.hpp>

namespace rogue {
class System;
struct PositionComp;
} // namespace rogue

namespace rogue {

class Level : public EventHubConnector {
public:
  static constexpr std::size_t LayerGroundIdx = 0;
  static constexpr std::size_t LayerGroundDecoIdx = 1;
  static constexpr std::size_t LayerWallsIdx = 2;
  static constexpr std::size_t LayerWallsDecoIdx = 3;
  static constexpr std::size_t LayerEntitiesIdx = 4;
  static constexpr std::size_t LayerObjectsIdx = 5;

  static const std::vector<std::string> LayerNames;

  // FIXME move to separate header and use in level gen
  static constexpr Tile EmptyTile = Tile{};
  static constexpr Tile WallTile = Tile{{'#'}};

public:
  Level(int LevelId, ymir::Size2d<int> Size);

  int getLevelId() const { return LevelId; }

  void setEventHub(EventHub *EH) override;

  // Returns false if game over
  bool update(bool IsTick);

  // FIXME decouple player from level
  void createPlayer();
  bool hasPlayer() const;
  void movePlayer(Level &From, ymir::Point2d<int> AtPos);
  void removePlayer();
  const entt::entity &getPlayer() const;

  /// Returns the entry position for the level
  /// \throws std::runtime_error if there is no entry
  ymir::Point2d<int> getLevelStartPos() const;

  /// Returns the exit position for the level
  /// \throws std::runtime_error if there is no exit
  ymir::Point2d<int> getLevelEndPos() const;

  std::vector<ymir::Point2d<int>>
  getAllNonBodyBlockedPosNextTo(ymir::Point2d<int> AtPos) const;

  std::optional<ymir::Point2d<int>>
  getNonBodyBlockedPosNextTo(ymir::Point2d<int> AtPos) const;

  bool canInteract(ymir::Point2d<int> Pos) const;
  std::vector<entt::entity> getInteractables(ymir::Point2d<int> Pos) const;

  bool isWallBlocked(ymir::Point2d<int> Pos) const;
  bool isLOSBlocked(ymir::Point2d<int> Pos) const;

  /// Returns true if the position is body blocked
  /// \param Hard If true, also checks for entities with collisions
  bool isBodyBlocked(ymir::Point2d<int> Pos, bool Hard = true) const;

  std::pair<ymir::Map<int, int>, std::vector<ymir::Point2d<int>>>
  getDijkstraMap(Tile Target, std::size_t Layer) const;

  void revealMap();
  const ymir::Map<bool, int> &getPlayerSeenMap() const;

  const entt::entity &getEntityAt(ymir::Point2d<int> AtPos) const;
  void updateEntityPosition(const entt::entity &Entity, PositionComp &PosComp,
                            ymir::Point2d<int> NextPos);

protected:
  void updatePlayerSeenMap();

  /// Updates entity positions based on entities with position and collision
  void updateEntityPosCache();

public: // FIXME
  ymir::LayeredMap<Tile> Map;
  entt::registry Reg;

private:
  int LevelId;
  std::vector<std::shared_ptr<System>> Systems;

  // FIXME decouple player from level
  entt::entity Player = entt::null;

  ymir::Map<entt::entity, int> EntityPosCache;
  ymir::Map<bool, int> PlayerSeenMap;
};

} // namespace rogue

#endif // #ifndef ROGUE_LEVEL_H