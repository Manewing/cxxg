#ifndef ROGUE_COMPONENTS_LOS_H
#define ROGUE_COMPONENTS_LOS_H

namespace rogue {

struct LineOfSightComp {
  unsigned LOSRange = 14;
  unsigned MaxLOSRange = 14;

  void reset() { LOSRange = MaxLOSRange; }
};

struct VisibleLOSComp {
  bool Temporary = false;
};

struct VisibleComp {
  bool IsVisible = true;
  bool Partially = false;
};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_LOS_H