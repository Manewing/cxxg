#ifndef ROGUE_COMPONENTS_COMBAT_H
#define ROGUE_COMPONENTS_COMBAT_H

#include <entt/entt.hpp>
#include <rogue/Components/Stats.h>
#include <ymir/Types.hpp>

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

struct DamageComp {
  entt::entity Source = entt::null;
  StatValue Damage = 100;
  int Hits = 1;
};

void createProjectile(entt::registry &Reg, entt::entity Source,
                      StatValue Damage, int Hits, ymir::Dir2d MoveDir, ymir::Point2d<int> Pos,
                      StatValue Agility = 100);

} // namespace rogue

#endif // ROGUE_COMPONENTS_COMBAT_H