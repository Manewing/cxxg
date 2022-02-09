#ifndef ROGUE_LEVEL_H
#define ROGUE_LEVEL_H

#include "Entity.h"
#include "Tile.h"
#include <memory>
#include <ymir/LayeredMap.hpp>
#include <vector>

class Level {
public:
  static constexpr std::size_t LayerGroundIdx = 0;
  static constexpr std::size_t LayerGroundDecoIdx = 1;
  static constexpr std::size_t LayerWallsIdx = 2;
  static constexpr std::size_t LayerObjectsIdx = 3;
  static constexpr std::size_t LayerWallsDecoIdx = 4;

  static constexpr Tile EmptyTile = Tile{};
  static constexpr Tile WallTile = Tile{{'#'}};
  static constexpr Tile StartTile = Tile{{'H'}};
  static constexpr Tile EndTile = Tile{{'<'}};

public:
  Level(const std::vector<std::string> &Layers, ymir::Size2d<int> Size);

  // TODO check that loaded level is valid
  // bool checkIsValid();

  // Returns false if game over
  bool update();

  void setPlayer(PlayerEntity *P = nullptr);
  PlayerEntity *getPlayer();

  ymir::Point2d<int> getPlayerStartPos() const;
  ymir::Point2d<int> getPlayerEndPos() const;

  std::vector<Entity *> getEntities();
  Entity* getEntityAt(ymir::Point2d<int> Pos);

  std::vector<ymir::Point2d<int>>
  getAllNonBodyBlockedPosNextTo(ymir::Point2d<int> AtPos) const;

  std::optional<ymir::Point2d<int>>
  getNonBodyBlockedPosNextTo(ymir::Point2d<int> AtPos) const;

  bool canInteract(ymir::Point2d<int> Pos) const;
  std::vector<Tile> getInteractables(ymir::Point2d<int> Pos) const;

  bool isLOSBlocked(ymir::Point2d<int> Pos) const;
  bool isBodyBlocked(ymir::Point2d<int> Pos) const;

  const ymir::Map<int, int> getPlayerDijkstraMap() const;

protected:
  void updatePlayerDijkstraMap();

public: // FIXME
  ymir::LayeredMap<Tile> Map;
  PlayerEntity *Player = nullptr;
  std::vector<std::shared_ptr<Entity>> Entities;

  ymir::Map<int, int> PlayerDijkstraMap;
};

#endif // #ifndef ROGUE_LEVEL_H