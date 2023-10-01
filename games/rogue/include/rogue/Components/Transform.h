#ifndef ROGUE_COMPONENTS_TRANSFORM_H
#define ROGUE_COMPONENTS_TRANSFORM_H

#include <rogue/Components/Stats.h>
#include <ymir/Types.hpp>

namespace rogue {

struct PositionComp {
  ymir::Point2d<int> Pos;

  // Allow implicit conversion
  PositionComp(const ymir::Point2d<int> &Pos = {}) : Pos(Pos) {}
  operator const ymir::Point2d<int> &() const { return Pos; }
};

struct MovementComp {
  static constexpr StatValue MoveAPCost = 10;

  /// Direction of movement
  ymir::Dir2d Dir = ymir::Dir2d::NONE;

  /// Clear movement after being applied
  bool Clear = true;

  /// True if the movement is flying
  bool Flying = false;

  /// Kill after hitting wall
  // FIXME move this
  bool KillOnWall = false;

  // Allow implicit conversion
  MovementComp(const ymir::Dir2d &Dir = ymir::Dir2d::NONE) : Dir(Dir) {}
  operator const ymir::Dir2d &() const { return Dir; }
};

struct CollisionComp {};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_TRANSFORM_H