#ifndef ROGUE_COMPONENTS_TRANSFORM_H
#define ROGUE_COMPONENTS_TRANSFORM_H

#include <ymir/Types.hpp>

struct PositionComp {
  ymir::Point2d<int> Pos;

  // Allow implicit conversion
  PositionComp(const ymir::Point2d<int> &Pos) : Pos(Pos) {}
  operator const ymir::Point2d<int> &() const { return Pos; }
};


// MovementComp?

#endif // #ifndef ROGUE_COMPONENTS_TRANSFORM_H