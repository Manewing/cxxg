#ifndef ROGUE_COMPONENTS_TRANSFORM_H
#define ROGUE_COMPONENTS_TRANSFORM_H

#include <ymir/Types.hpp>

namespace rogue {

struct PositionComp {
  ymir::Point2d<int> Pos;

  // Allow implicit conversion
  PositionComp(const ymir::Point2d<int> &Pos = {}) : Pos(Pos) {}
  operator const ymir::Point2d<int> &() const { return Pos; }
};

struct MovementComp {
  ymir::Dir2d Dir = ymir::Dir2d::NONE;

  // Allow implicit conversion
  MovementComp(const ymir::Dir2d &Dir = ymir::Dir2d::NONE) : Dir(Dir) {}
  operator const ymir::Dir2d &() const { return Dir; }
};

struct CollisionComp {};

}

#endif // #ifndef ROGUE_COMPONENTS_TRANSFORM_H