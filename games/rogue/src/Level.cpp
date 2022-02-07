#include "Level.h"

Level::Level(const std::vector<std::string> &Layers, ymir::Size2d<int> Size)
    : Map(Layers, Size) {}

bool Level::isBodyBlocked(ymir::Point2d<int> Pos) {
  return Map.get(LayerWallsIdx).getTile(Pos).Char != EmptyTile.Char;
}