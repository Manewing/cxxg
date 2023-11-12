#include <rogue/Components/Combat.h>
#include <rogue/Components/Helpers.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>

namespace rogue {

StatValue RangedAttackComp::getPhysEffectiveDamage(const StatPoints *SP) const {
  if (!SP || PhysDamage == 0) {
    return PhysDamage;
  }
  auto Dex = StatValue(SP->Dex);
  return (PhysDamage + Dex) * (100.0 + Dex) / 100.0;
}

StatValue
RangedAttackComp::getMagicEffectiveDamage(const StatPoints *SP) const {
  if (!SP || MagicDamage == 0) {
    return MagicDamage;
  }
  auto Int = StatValue(SP->Int);
  return (MagicDamage + Int) * (100.0 + Int) / 100.0;
}

StatValue MeleeAttackComp::getPhysEffectiveDamage(const StatPoints *SP) const {
  if (!SP || PhysDamage == 0) {
    return PhysDamage;
  }
  auto Str = StatValue(SP->Str);
  return (PhysDamage + Str) * (100.0 + Str) / 100.0;
}

StatValue MeleeAttackComp::getMagicEffectiveDamage(const StatPoints *SP) const {
  if (!SP || MagicDamage == 0) {
    return MagicDamage;
  }
  auto Int = StatValue(SP->Int);
  return (MagicDamage + Int) * (100.0 + Int) / 100.0;
}

// FIXME include collision?
using ProjectileCompList = ComponentList<DamageComp, PositionComp, AgilityComp,
                                         TileComp, MovementComp>;

static constexpr auto ProjectileTile =
    Tile{{'*', cxxg::types::RgbColor{255, 65, 0}}};

void createProjectile(entt::registry &Reg, const DamageComp &DC,
                      ymir::Point2d<int> Pos, ymir::Point2d<int> TargetPos,
                      StatValue Agility) {
  auto E = Reg.create();
  Reg.emplace<DamageComp>(E, DC);
  Reg.emplace<PositionComp>(E, Pos);
  Reg.emplace<AgilityComp>(E, Agility);
  Reg.emplace<TileComp>(E, ProjectileTile);
  VectorMovementComp VMC;
  VMC.Flying = true;
  VMC.KillOnWall = true;
  VMC.Vector = (TargetPos - Pos).to<float>();
  VMC.LastPos = Pos.to<float>();
  Reg.emplace<VectorMovementComp>(E, VMC);
}

} // namespace rogue