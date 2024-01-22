#include <rogue/Components/Combat.h>
#include <rogue/Components/Helpers.h>
#include <rogue/Components/LOS.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>

namespace rogue {

StatValue RangedAttackComp::getPhysEffectiveDamage(const StatPoints &SP) const {
  if (PhysDamage == 0) {
    return 0;
  }
  auto Dex = StatValue(SP.Dex);
  return (PhysDamage + Dex) * (100.0 + Dex) / 100.0;
}

StatValue
RangedAttackComp::getMagicEffectiveDamage(const StatPoints &SP) const {
  if (MagicDamage == 0) {
    return 0;
  }
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
  if (PhysDamage == 0) {
    return 0;
  }
  auto Str = StatValue(SP.Str);
  return (PhysDamage + Str) * (100.0 + Str) / 100.0;
}

StatValue MeleeAttackComp::getMagicEffectiveDamage(const StatPoints &SP) const {
  if (MagicDamage == 0) {
    return 0;
  }
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

} // namespace rogue