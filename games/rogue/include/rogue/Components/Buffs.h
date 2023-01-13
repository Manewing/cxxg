#ifndef ROGUE_COMPONENTS_BUFFS_H
#define ROGUE_COMPONENTS_BUFFS_H

#include <rogue/Components/Stats.h>

namespace rogue {

// TODO:
//  Buffs:
//      - Stat buff, Stat.X + Y
//  Debuffs:
//      - Burning
//      - Slow
//      - Blinded

struct PoisonDebuffComp {};

struct BleedingDebuffComp {};

struct HealthRegenBuffComp {
  /// Ticks left before buff is gone
  unsigned TicksLeft = 10;
  StatValue RegenAmount = 0.1;
};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_BUFFS_H