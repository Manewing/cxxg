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

struct RangedAttackComp {
  StatValue PhysDamage = 10;
  StatValue MagicDamage = 0;
  StatValue APCost = 15;

  /// Dexterity increases physical damage by 1 and by 1% per point
  StatValue getPhysEffectiveDamage(const StatPoints *SP = nullptr) const;

  /// Intelligence increases magic damage by 1 and by 1% per point
  StatValue getMagicEffectiveDamage(const StatPoints *SP = nullptr) const;
};

struct MeleeAttackComp {
  StatValue PhysDamage = 10;
  StatValue MagicDamage = 0;
  StatValue APCost = 10;

  /// Strength increases melee damage by 1 and by 1% per point
  StatValue getPhysEffectiveDamage(const StatPoints *SP = nullptr) const;

  /// Intelligence increases magic damage by 1 and by 1% per point
  StatValue getMagicEffectiveDamage(const StatPoints *SP = nullptr) const;
};

struct DamageComp {
  entt::entity Source = entt::null;
  StatValue PhysDamage = 10;
  StatValue MagicDamage = 0;
  int Hits = 1;

  StatValue total() const { return PhysDamage + MagicDamage; }
};

void createProjectile(entt::registry &Reg, const DamageComp &DC,
                      ymir::Point2d<int> Pos, ymir::Point2d<int> TargetPos,
                      StatValue Agility = 100);

} // namespace rogue

#endif // ROGUE_COMPONENTS_COMBAT_H