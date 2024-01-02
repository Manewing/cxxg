#include <rogue/Components/Combat.h>
#include <rogue/Components/Helpers.h>
#include <rogue/Components/LOS.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>

namespace rogue {

StatValue RangedAttackComp::getPhysEffectiveDamage(const StatPoints &SP) const {
  auto Dex = StatValue(SP.Dex);
  return (PhysDamage + Dex) * (100.0 + Dex) / 100.0;
}

StatValue
RangedAttackComp::getMagicEffectiveDamage(const StatPoints &SP) const {
  auto Int = StatValue(SP.Int);
  return (MagicDamage + Int) * (100.0 + Int) / 100.0;
}

RangedAttackComp RangedAttackComp::getEffective(const StatsComp *SC) const {
  RangedAttackComp RAC = *this;
  if (!SC) {
    return RAC;
  }
  auto Eff = SC->effective();
  RAC.PhysDamage = getPhysEffectiveDamage(Eff);
  RAC.MagicDamage = getMagicEffectiveDamage(Eff);
  return RAC;
}

StatValue MeleeAttackComp::getPhysEffectiveDamage(const StatPoints &SP) const {
  auto Str = StatValue(SP.Str);
  return (PhysDamage + Str) * (100.0 + Str) / 100.0;
}

StatValue MeleeAttackComp::getMagicEffectiveDamage(const StatPoints &SP) const {
  auto Int = StatValue(SP.Int);
  return (MagicDamage + Int) * (100.0 + Int) / 100.0;
}

MeleeAttackComp MeleeAttackComp::getEffective(const StatsComp *SC) const {
  MeleeAttackComp MAC = *this;
  if (!SC) {
    return MAC;
  }
  auto Eff = SC->effective();
  MAC.PhysDamage = getPhysEffectiveDamage(Eff);
  MAC.MagicDamage = getMagicEffectiveDamage(Eff);
  return MAC;
}

// FIXME include collision?
using ProjectileCompList = ComponentList<DamageComp, PositionComp, AgilityComp,
                                         TileComp, MovementComp>;

static constexpr auto TempDamageTile =
    Tile{{'*', cxxg::types::RgbColor{175, 175, 175}}};
static constexpr auto ProjectileTile =
    Tile{{'*', cxxg::types::RgbColor{255, 65, 0}}};

void createTempDamage(entt::registry &Reg, const DamageComp &DC,
                      ymir::Point2d<int> Pos) {
  auto E = Reg.create();
  Reg.emplace<DamageComp>(E, DC).Ticks = 1;
  Reg.emplace<PositionComp>(E, Pos);
  Reg.emplace<TileComp>(E, TempDamageTile);
  Reg.emplace<NameComp>(E, "Damage");
  Reg.emplace<VisibleComp>(E);
}

void createProjectile(entt::registry &Reg, const DamageComp &DC,
                      ymir::Point2d<int> Pos, ymir::Point2d<int> TargetPos,
                      StatValue Agility) {
  auto E = Reg.create();
  Reg.emplace<DamageComp>(E, DC);
  Reg.emplace<PositionComp>(E, Pos);
  Reg.emplace<AgilityComp>(E, Agility);
  Reg.emplace<TileComp>(E, ProjectileTile);
  Reg.emplace<NameComp>(E, "Projectile");
  VectorMovementComp VMC;
  VMC.Flying = true;
  VMC.KillOnWall = true;
  VMC.Vector = (TargetPos - Pos).to<float>();
  VMC.LastPos = Pos.to<float>();
  Reg.emplace<VectorMovementComp>(E, VMC);
  Reg.emplace<VisibleComp>(E);
}

} // namespace rogue