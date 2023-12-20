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

} // namespace rogue