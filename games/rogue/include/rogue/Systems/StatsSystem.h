#ifndef ROGUE_SYSTEMS_STATS_SYSTEM_H
#define ROGUE_SYSTEMS_STATS_SYSTEM_H

#include <rogue/Systems/System.h>

namespace rogue {

class StatsSystem : public System {
public:
  using System::System;
  bool needsTick() const final { return false; }
  void update() override;
};

/// Updates stats that are providing time based modifiers, needs to run after
/// StatsSystem
class TimedStatsSystem : public System {
public:
  using System::System;
  bool needsTick() const final { return true; }
  void update() override;
};

} // namespace rogue

#endif // #ifndef ROGUE_SYSTEMS_REGEN_SYSTEM_H