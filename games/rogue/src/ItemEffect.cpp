#include <rogue/ItemEffect.h>

namespace rogue {

HealItemEffect::HealItemEffect(StatValue Amount) : Amount(Amount) {}

bool HealItemEffect::canApplyTo(const entt::entity &Et,
                                entt::registry &Reg) const {
  return Reg.any_of<HealthComp>(Et);
}

void HealItemEffect::applyTo(const entt::entity &Et,
                             entt::registry &Reg) const {
  Reg.get<HealthComp>(Et).restore(Amount);
}

DamageItemEffect::DamageItemEffect(StatValue Amount) : Amount(Amount) {}

bool DamageItemEffect::canApplyTo(const entt::entity &Et,
                                  entt::registry &Reg) const {
  return Reg.any_of<HealthComp>(Et);
}

void DamageItemEffect::applyTo(const entt::entity &Et,
                               entt::registry &Reg) const {
  Reg.get<HealthComp>(Et).reduce(Amount);
}

} // namespace rogue