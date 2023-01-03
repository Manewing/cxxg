#ifndef ROGUE_LEVEL_H
#define ROGUE_LEVEL_H

#include "Entity.h"
#include "EventHub.h"
#include "Tile.h"
#include <entt/entt.hpp>
#include <memory>
#include <vector>
#include <ymir/LayeredMap.hpp>

class System;
struct PositionComp;

class Level : public EventHubConnector {
public:
  static constexpr std::size_t LayerGroundIdx = 0;
  static constexpr std::size_t LayerGroundDecoIdx = 1;
  static constexpr std::size_t LayerWallsIdx = 2;
  static constexpr std::size_t LayerWallsDecoIdx = 3;

  static constexpr Tile EmptyTile = Tile{};
  static constexpr Tile WallTile = Tile{{'#'}};
  static constexpr Tile StartTile = Tile{{'H'}};
  static constexpr Tile EndTile = Tile{{'<'}};

public:
  Level(const std::vector<std::string> &Layers, ymir::Size2d<int> Size);

  void setEventHub(EventHub *EH) override;

  // Returns false if game over
  bool update();

  void createPlayer();
  void movePlayer(Level &From);
  void removePlayer();
  const entt::entity &getPlayer() const;

  ymir::Point2d<int> getPlayerStartPos() const;
  ymir::Point2d<int> getPlayerEndPos() const;

  std::vector<ymir::Point2d<int>>
  getAllNonBodyBlockedPosNextTo(ymir::Point2d<int> AtPos) const;

  std::optional<ymir::Point2d<int>>
  getNonBodyBlockedPosNextTo(ymir::Point2d<int> AtPos) const;

  bool canInteract(ymir::Point2d<int> Pos) const;
  std::vector<Tile> getInteractables(ymir::Point2d<int> Pos) const;

  bool isLOSBlocked(ymir::Point2d<int> Pos) const;
  bool isBodyBlocked(ymir::Point2d<int> Pos) const;

  ymir::Map<int, int> getDijkstraMap(Tile Target, std::size_t Layer) const;

  const ymir::Map<int, int> &getPlayerDijkstraMap() const;
  const ymir::Map<bool, int> &getPlayerSeenMap() const;

  const entt::entity &getEntityAt(ymir::Point2d<int> AtPos) const;
  void updateEntityPosition(const entt::entity &Entity, PositionComp &PosComp,
                            ymir::Point2d<int> NextPos);

protected:
  void updatePlayerDijkstraMap();
  void updatePlayerSeenMap();

  void updateEntityPosCache();

public: // FIXME
  ymir::LayeredMap<Tile> Map;
  std::vector<std::shared_ptr<Entity>> Entities;

  entt::registry Reg;

private:
  std::vector<std::shared_ptr<System>> Systems;

  entt::entity Player = entt::null;

  ymir::Map<entt::entity, int> EntityPosCache;
  ymir::Map<int, int> PlayerDijkstraMap;
  ymir::Map<bool, int> PlayerSeenMap;
};

#endif // #ifndef ROGUE_LEVEL_H