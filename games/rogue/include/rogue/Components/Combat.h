#ifndef ROGUE_COMPONENTS_COMBAT_H
#define ROGUE_COMPONENTS_COMBAT_H

#include <entt/entt.hpp>
#include <rogue/Components/Stats.h>
#include <ymir/Types.hpp>

namespace rogue {

/// Component that indicates the entity wants to execute a combat action
struct CombatActionComp {
  /// If set, indicates an attack towards the given entity
  entt::entity Target = entt::null;

  /// If set, indicates a ranged attack towards the given position
  std::optional<ymir::Point2d<int>> RangedPos = std::nullopt;
};

/// Indicates that the entity is attacking another entity
struct CombatAttackComp {
  /// The entity that is currently targeted by the attack
  entt::entity Target = entt::null;
};

/// Indicates that the entity is the target of an attack
struct CombatTargetComp {
  /// The entity that is currently attacking this entity
  entt::entity Attacker = entt::null;
};

struct RangedAttackComp {
  StatValue PhysDamage = 10;
  StatValue MagicDamage = 0;
  StatValue APCost = 15;
  StatValue ManaCost = 0;

  /// Dexterity increases physical damage by 1 and by 1% per point
  StatValue getPhysEffectiveDamage(const StatPoints &SP) const;

  /// Intelligence increases magic damage by 1 and by 1% per point
  StatValue getMagicEffectiveDamage(const StatPoints &SP) const;

  /// Returns the effective ranged attack component based on the given stats
  RangedAttackComp getEffective(const StatsComp *SC) const;
};

struct MeleeAttackComp {
  StatValue PhysDamage = 10;
  StatValue MagicDamage = 0;
  StatValue APCost = 10;
  StatValue ManaCost = 0;

  /// Strength increases melee damage by 1 and by 1% per point
  StatValue getPhysEffectiveDamage(const StatPoints &SP) const;

  /// Intelligence increases magic damage by 1 and by 1% per point
  StatValue getMagicEffectiveDamage(const StatPoints &SP) const;

  /// Returns the effective melee attack component based on the given stats
  MeleeAttackComp getEffective(const StatsComp *SP) const;
};

struct DamageComp {
  entt::entity Source = entt::null;
  StatValue PhysDamage = 0;
  StatValue MagicDamage = 0;
  int Hits = 1;

  /// Number of ticks before the damage component will be removed
  unsigned Ticks = -1U;

  StatValue total() const { return PhysDamage + MagicDamage; }
};

void createTempDamage(entt::registry &Reg, const DamageComp &DC,
                      ymir::Point2d<int> Pos);

void createProjectile(entt::registry &Reg, const DamageComp &DC,
                      ymir::Point2d<int> Pos, ymir::Point2d<int> TargetPos,
                      StatValue Agility = 100);

} // namespace rogue

#endif // ROGUE_COMPONENTS_COMBAT_H