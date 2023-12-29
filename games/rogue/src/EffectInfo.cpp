#include <rogue/Components/Stats.h>
#include <rogue/EffectInfo.h>
#include <rogue/ItemEffect.h>
#include <sstream>

namespace rogue {

void EffectAttributes::addFrom(const EffectAttributes &Other) {
  Flags = Flags | Other.Flags;
  APCost += Other.APCost;
  ManaCost += Other.ManaCost;
  HealthCost += Other.HealthCost;
}

void EffectAttributes::updateCostsFrom(const EffectAttributes &Other) {
  APCost = std::max(APCost, Other.APCost);
  ManaCost = std::max(ManaCost, Other.ManaCost);
  HealthCost = std::max(HealthCost, Other.HealthCost);
}

bool EffectAttributes::checkCosts(const entt::entity &Entity,
                                  entt::registry &Reg) const {
  // If there is a check if there is an agility component, we do not check if
  // there is enough AP as we allow it to go negative
  if (APCost > 0) {
    auto *AG = Reg.try_get<AgilityComp>(Entity);
    if (!AG) {
      return false;
    }
  }

  // Check that there is a mana component and that there is enough mana
  if (ManaCost > 0) {
    auto *MP = Reg.try_get<ManaComp>(Entity);
    if (!MP || !MP->hasAmount(ManaCost)) {
      return false;
    }
  }

  // Check that there is a health component and that there is enough health
  if (HealthCost > 0) {
    auto *HP = Reg.try_get<HealthComp>(Entity);
    if (!HP || !HP->hasAmount(HealthCost)) {
      return false;
    }
  }

  return true;
}

void EffectAttributes::applyCosts(const entt::entity &Entity,
                                  entt::registry &Reg) const {
  if (APCost > 0) {
    auto &AG = Reg.get<AgilityComp>(Entity);
    AG.spendAP(APCost);
  }

  if (ManaCost > 0) {
    auto &MP = Reg.get<ManaComp>(Entity);
    assert(MP.hasAmount(ManaCost));
    MP.reduce(ManaCost);
  }

  if (HealthCost > 0) {
    auto &HP = Reg.get<HealthComp>(Entity);
    assert(HP.hasAmount(HealthCost));
    HP.reduce(HealthCost);
  }
}

std::ostream &operator<<(std::ostream &OS, const EffectAttributes &Attr) {
  OS << "EffectAttributes{Flags: " << Attr.Flags;
  if (Attr.APCost) {
    OS << ", APCost: " << Attr.APCost;
  }
  if (Attr.ManaCost) {
    OS << ", ManaCost: " << Attr.ManaCost;
  }
  if (Attr.HealthCost) {
    OS << ", HealthCost: " << Attr.HealthCost;
  }
  OS << "}";
  return OS;
}

bool EffectInfo::canApplyTo(const entt::entity &SrcEt,
                            const entt::entity &DstEt, entt::registry &Reg,
                            CapabilityFlags Flags) const {
  if (!(Attributes.Flags & Flags)) {
    return false;
  }

  if (!Attributes.checkCosts(SrcEt, Reg)) {
    return false;
  }

  return Effect->canApplyTo(DstEt, Reg);
}

void EffectInfo::applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                         entt::registry &Reg, CapabilityFlags Flags) const {
  if (!canApplyTo(SrcEt, DstEt, Reg, Flags)) {
    std::stringstream SS;
    SS << "Can't apply: " << *this;
    throw std::runtime_error(SS.str());
  }
  Attributes.applyCosts(SrcEt, Reg);
  Effect->applyTo(DstEt, Reg);
}

bool EffectInfo::canRemoveFrom(const entt::entity &SrcEt,
                               const entt::entity &DstEt, entt::registry &Reg,
                               CapabilityFlags Flags) const {
  if (!(Attributes.Flags & Flags)) {
    return false;
  }
  (void)SrcEt;
  return Effect->canRemoveFrom(DstEt, Reg);
}

void EffectInfo::removeFrom(const entt::entity &SrcEt,
                            const entt::entity &DstEt, entt::registry &Reg,
                            CapabilityFlags Flags) const {
  if (!canRemoveFrom(SrcEt, DstEt, Reg, Flags)) {
    std::stringstream SS;
    SS << "Can't remove: " << *this;
    throw std::runtime_error(SS.str());
  }
  Effect->removeFrom(DstEt, Reg);
}

std::ostream &operator<<(std::ostream &OS, const EffectInfo &Info) {
  OS << "EffectInfo{Attributes: " << Info.Attributes
     << ", Effect: " << Info.Effect->getName() << "}";
  return OS;
}

} // namespace rogue