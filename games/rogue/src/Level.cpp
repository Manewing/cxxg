#include "Level.h"

Level::Level(const std::vector<std::string> &Layers, ymir::Size2d<int> Size)
    : Map(Layers, Size) {}

void Level::setPlayer(PlayerEntity *P) { this->Player = P; }

PlayerEntity *Level::getPlayer() { return Player; }

ymir::Point2d<int> Level::getPlayerStartPos() const {
  auto AllStartPos = Map.get(LayerObjectsIdx).findTiles(StartTile);
  for (auto StartPos : AllStartPos) {
    auto Pos = getUnblockedPosNextTo(StartPos);
    if (Pos) {
      return *Pos;
    }
  }
  throw std::runtime_error("Could not find start position for player in level");
}

ymir::Point2d<int> Level::getPlayerEndPos() const {
  auto AllEndPos = Map.get(LayerObjectsIdx).findTiles(EndTile);
  for (auto EndPos : AllEndPos) {
    auto Pos = getUnblockedPosNextTo(EndPos);
    if (Pos) {
      return *Pos;
    }
  }
  throw std::runtime_error("Could not find end position for player in level");
}

std::vector<Entity*> Level::getEntities() {
  std::vector<Entity*> AllEntities;
  AllEntities.reserve(Entities.size() + 1);
  for (auto &Entity : Entities) {
    AllEntities.push_back(Entity.get());
  }
  if (Player) {
    AllEntities.push_back(Player);
  }
  return AllEntities;
}

std::optional<ymir::Point2d<int>>
Level::getUnblockedPosNextTo(ymir::Point2d<int> AtPos) const {
  std::optional<ymir::Point2d<int>> UnblockedPos;
  Map.get(LayerObjectsIdx)
      .checkNeighbors(
          AtPos,
          [this, &UnblockedPos](auto Pos, auto) {
            if (!isBodyBlocked(Pos)) {
              UnblockedPos = Pos;
              return false;
            }
            return true;
          },
          ymir::EightTileDirections<int>());
  return UnblockedPos;
}

bool Level::canInteract(ymir::Point2d<int> AtPos) const {
  bool FoundObject = false;
  Map.get(LayerObjectsIdx)
      .checkNeighbors(
          AtPos,
          [&FoundObject](auto, auto Tile) {
            if (Tile != EmptyTile) {
              FoundObject = true;
              return false;
            }
            return true;
          },
          ymir::FourTileDirections<int>());
  return FoundObject;
}

std::vector<Tile> Level::getInteractables(ymir::Point2d<int> AtPos) const {
  std::vector<Tile> Objects;
  Objects.reserve(4);
  Map.get(LayerObjectsIdx)
      .checkNeighbors(
          AtPos,
          [&Objects](auto, auto Tile) {
            if (Tile != EmptyTile) {
              Objects.push_back(Tile);
            }
            return true;
          },
          ymir::FourTileDirections<int>());
  return Objects;
}

bool Level::isLOSBlocked(ymir::Point2d<int> Pos) const {
  return Map.get(LayerWallsIdx).getTile(Pos) != EmptyTile;
}

bool Level::isBodyBlocked(ymir::Point2d<int> Pos) const {
  return Map.get(LayerWallsIdx).getTile(Pos) != EmptyTile ||
         Map.get(LayerObjectsIdx).getTile(Pos) != EmptyTile;
}