#ifndef ROGUE_LEVEL_H
#define ROGUE_LEVEL_H

#include <cxxg/Types.h>
#include <ymir/LayeredMap.hpp>

class Level {
public:
  using TileType = cxxg::types::ColoredChar;

public:
  static constexpr std::size_t LayerGroundIdx = 0;
  static constexpr std::size_t LayerGroundDecoIdx = 1;
  static constexpr std::size_t LayerWallsIdx = 2;
  static constexpr std::size_t LayerWallsDecoIdx = 3;

  static constexpr TileType EmptyTile = TileType();

public:
  Level(const std::vector<std::string> &Layers, ymir::Size2d<int> Size);

  // TODO check that loaded level is valid
  // bool checkIsValid();

  bool isBodyBlocked(ymir::Point2d<int> Pos);

  ymir::LayeredMap<cxxg::types::ColoredChar> Map;
};

#endif // #ifndef ROGUE_LEVEL_H