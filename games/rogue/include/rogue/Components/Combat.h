#ifndef ROGUE_COMPONENTS_COMBAT_H
#define ROGUE_COMPONENTS_COMBAT_H

#include <entt/entt.hpp>
#include <rogue/Components/Stats.h>

namespace rogue {

struct CombatComp {
  entt::entity Target = entt::null;
};

struct MeleeAttackComp {
  StatValue Damage = 10;
  StatValue APCost = 10;

  /// Strength increases melee damage by 1 and by 1% per point
  StatValue getEffectiveDamage(StatPoints SrcStats) const {
    auto Str = StatValue(SrcStats.Str);
    return (Damage + Str) * (100.0 + Str) / 100.0;
  }

  StatValue getEffectiveDamage(StatsComp *SrcSC) const {
    if (!SrcSC) {
      return Damage;
    }
    return getEffectiveDamage(SrcSC->effective());
  }
};

} // namespace rogue

#endif // ROGUE_COMPONENTS_COMBAT_H