#include <rogue/Components/Transform.h>
#include <rogue/ItemEffectImpl.h>
#include <sstream>

namespace rogue {

std::shared_ptr<ItemEffect> SetMeleeCompEffect::clone() const {
  return std::make_shared<SetMeleeCompEffect>(*this);
}

std::string SetMeleeCompEffect::getName() const { return "Melee Attack"; }

namespace {

std::string getDmgDescription(StatValue Phys, StatValue Magic,
                              const std::string &Type) {
  if (Phys == 0 && Magic == 0) {
    return "no dmg.";
  }

  std::stringstream SS;
  SS << Type << " ";
  const char *Pred = "";
  if (Phys > 0) {
    SS << Pred << Phys << " phys.";
    Pred = " ";
  }
  if (Magic > 0) {
    SS << Pred << Magic << " magic";
    Pred = " ";
  }
  SS << " dmg.";
  return SS.str();
}

} // namespace

std::string SetMeleeCompEffect::getDescription() const {
  return getDmgDescription(Comp.PhysDamage, Comp.MagicDamage, "melee");
}

std::shared_ptr<ItemEffect> SetRangedCompEffect::clone() const {
  return std::make_shared<SetRangedCompEffect>(*this);
}

std::string SetRangedCompEffect::getName() const { return "Ranged Attack"; }

std::string SetRangedCompEffect::getDescription() const {
  return getDmgDescription(Comp.PhysDamage, Comp.MagicDamage, "ranged");
}

std::shared_ptr<ItemEffect> RemovePoisonEffect::clone() const {
  return std::make_shared<RemovePoisonEffect>(*this);
}

std::string RemovePoisonEffect::getName() const {
  return "Remove poison effect";
}

std::string RemovePoisonEffect::getDescription() const {
  return "Removes poison effect";
}

std::shared_ptr<ItemEffect> RemovePoisonDebuffEffect::clone() const {
  return std::make_shared<RemovePoisonDebuffEffect>(*this);
}

std::string RemovePoisonDebuffEffect::getName() const {
  return "Remove poison debuff";
}

std::string RemovePoisonDebuffEffect::getDescription() const {
  return "Removes poison debuff";
}

std::shared_ptr<ItemEffect> SweepingStrikeEffect::clone() const {
  return std::make_shared<SweepingStrikeEffect>(*this);
}

std::string SweepingStrikeEffect::getName() const { return "Sweeping Strike"; }

std::string SweepingStrikeEffect::getDescription() const {
  return "Hits all surrounding enemies.";
}

bool SweepingStrikeEffect::canApplyTo(const entt::entity &Et,
                                      entt::registry &Reg) const {
  return Reg.all_of<PositionComp, MeleeAttackComp>(Et);
}

void SweepingStrikeEffect::applyTo(const entt::entity &Et,
                                   entt::registry &Reg) const {
  auto &PC = Reg.get<PositionComp>(Et);

  // Melee is always possible, used default values for damage if not set
  const MeleeAttackComp DefaultMA = {
      .PhysDamage = 1, .MagicDamage = 0, .APCost = 10};
  MeleeAttackComp MA = DefaultMA;
  auto *AMA = Reg.try_get<MeleeAttackComp>(Et);
  if (AMA) {
    MA = *AMA;
  }

  DamageComp DC;
  DC.Source = Et;
  if (auto *SC = Reg.try_get<StatsComp>(Et)) {
    auto SP = SC->effective();
    DC.PhysDamage = MA.getPhysEffectiveDamage(&SP);
    DC.MagicDamage = MA.getMagicEffectiveDamage(&SP);
  } else {
    DC.PhysDamage = MA.getPhysEffectiveDamage();
    DC.MagicDamage = MA.getMagicEffectiveDamage();
  }

  for (const auto &Dir : ymir::EightTileDirections<int>::get()) {
    createTempDamage(Reg, DC, PC.Pos + Dir);
  }
}

} // namespace rogue