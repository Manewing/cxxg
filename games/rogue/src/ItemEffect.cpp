#include <rogue/Components/Items.h>
#include <rogue/ItemDatabase.h>
#include <rogue/ItemEffect.h>
#include <sstream>

namespace rogue {

void ItemEffect::markCombat(const entt::entity &SrcEt,
                            const entt::entity &DstEt, entt::registry &Reg) {
  BuffApplyHelperBase::markCombat(SrcEt, DstEt, Reg);
}

std::shared_ptr<ItemEffect> NullEffect::clone() const {
  return std::make_shared<NullEffect>(*this);
}

std::string NullEffect::getName() const { return "Null Effect"; }

std::string NullEffect::getDescription() const { return "Nothing"; }

HealItemEffect::HealItemEffect(StatValue Amount) : Amount(Amount) {}

std::shared_ptr<ItemEffect> HealItemEffect::clone() const {
  return std::make_shared<HealItemEffect>(*this);
}

std::string HealItemEffect::getName() const { return "Heal"; }

std::string HealItemEffect::getDescription() const {
  std::stringstream SS;
  SS << "Heals for " << Amount;
  return SS.str();
}

bool HealItemEffect::canAddFrom(const ItemEffect &Other) const {
  if (auto *OtherHeal = dynamic_cast<const HealItemEffect *>(&Other)) {
    // Currently anything can be added may make sense to restrict this
    (void)OtherHeal;
    return true;
  }
  return false;
}

void HealItemEffect::addFrom(const ItemEffect &Other) {
  if (auto *OtherHeal = dynamic_cast<const HealItemEffect *>(&Other)) {
    Amount += OtherHeal->Amount;
  }
}

bool HealItemEffect::canApplyTo(const entt::entity &, const entt::entity &DstEt,
                                entt::registry &Reg) const {
  return Reg.any_of<HealthComp>(DstEt);
}

void HealItemEffect::applyTo(const entt::entity &, const entt::entity &DstEt,
                             entt::registry &Reg) const {
  Reg.get<HealthComp>(DstEt).restore(Amount);
}

DamageItemEffect::DamageItemEffect(StatValue Amount) : Amount(Amount) {}

std::shared_ptr<ItemEffect> DamageItemEffect::clone() const {
  return std::make_shared<DamageItemEffect>(*this);
}

std::string DamageItemEffect::getName() const { return "Damage"; }

std::string DamageItemEffect::getDescription() const {
  std::stringstream SS;
  SS << "Deals " << Amount << " damage";
  return SS.str();
}

bool DamageItemEffect::canAddFrom(const ItemEffect &Other) const {
  if (auto *OtherDamage = dynamic_cast<const DamageItemEffect *>(&Other)) {
    // Currently anything can be added may make sense to restrict this
    (void)OtherDamage;
    return true;
  }
  return false;
}

void DamageItemEffect::addFrom(const ItemEffect &Other) {
  if (auto *OtherDamage = dynamic_cast<const DamageItemEffect *>(&Other)) {
    Amount += OtherDamage->Amount;
  }
}

bool DamageItemEffect::canApplyTo(const entt::entity &,
                                  const entt::entity &DstEt,
                                  entt::registry &Reg) const {
  return Reg.any_of<HealthComp>(DstEt);
}

void DamageItemEffect::applyTo(const entt::entity &SrcEt,
                               const entt::entity &DstEt,
                               entt::registry &Reg) const {
  Reg.get<HealthComp>(DstEt).reduce(Amount);
  markCombat(SrcEt, DstEt, Reg);
}

DismantleEffect::DismantleEffect(const ItemDatabase &DB,
                                 std::vector<DismantleResult> Results)
    : ItemDb(DB), Results(std::move(Results)) {}

std::shared_ptr<ItemEffect> DismantleEffect::clone() const {
  return std::make_shared<DismantleEffect>(*this);
}

std::string DismantleEffect::getName() const { return "Dismantle"; }

std::string DismantleEffect::getDescription() const {
  std::stringstream SS;
  SS << "Dismantles into:\n";
  for (const auto &Result : Results) {
    SS << "  " << Result.Amount << "x "
       << ItemDb.getItemProto(Result.ItemId).Name << "\n";
  }
  return SS.str();
}

bool DismantleEffect::canAddFrom(const ItemEffect &Other) const {
  if (auto *OtherDismantle = dynamic_cast<const DismantleEffect *>(&Other)) {
    // Currently anything can be added may make sense to restrict this
    (void)OtherDismantle;
    return true;
  }
  return false;
}

void DismantleEffect::addFrom(const ItemEffect &Other) {
  if (auto *OtherDismantle = dynamic_cast<const DismantleEffect *>(&Other)) {
    for (const auto &Result : OtherDismantle->Results) {
      Results.push_back(Result);
    }
  }
}

bool DismantleEffect::canApplyTo(const entt::entity &,
                                 const entt::entity &DstEt,
                                 entt::registry &Reg) const {
  return Reg.all_of<InventoryComp>(DstEt);
}

void DismantleEffect::applyTo(const entt::entity &, const entt::entity &DstEt,
                              entt::registry &Reg) const {
  auto &Inv = Reg.get<InventoryComp>(DstEt);
  for (const auto &Result : Results) {
    Inv.Inv.addItem(ItemDb.createItem(Result.ItemId, Result.Amount));
  }
  // FIXME we need to make item entities and destroy the item here in the
  // registry (currently handled in UI)
}

std::string ApplyBuffItemEffectBase::getName() const {
  return "Apply " + getBuff().getName();
}

std::string ApplyBuffItemEffectBase::getDescription() const {
  return getBuff().getDescription();
}

} // namespace rogue