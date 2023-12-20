#include <rogue/Components/Items.h>
#include <rogue/ItemDatabase.h>
#include <rogue/ItemEffect.h>

namespace rogue {

std::shared_ptr<ItemEffect> NullEffect::clone() const {
  return std::make_shared<NullEffect>(*this);
}

HealItemEffect::HealItemEffect(StatValue Amount) : Amount(Amount) {}

std::shared_ptr<ItemEffect> HealItemEffect::clone() const {
  return std::make_shared<HealItemEffect>(*this);
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

bool HealItemEffect::canApplyTo(const entt::entity &Et,
                                entt::registry &Reg) const {
  return Reg.any_of<HealthComp>(Et);
}

void HealItemEffect::applyTo(const entt::entity &Et,
                             entt::registry &Reg) const {
  Reg.get<HealthComp>(Et).restore(Amount);
}

DamageItemEffect::DamageItemEffect(StatValue Amount) : Amount(Amount) {}

std::shared_ptr<ItemEffect> DamageItemEffect::clone() const {
  return std::make_shared<DamageItemEffect>(*this);
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

bool DamageItemEffect::canApplyTo(const entt::entity &Et,
                                  entt::registry &Reg) const {
  return Reg.any_of<HealthComp>(Et);
}

void DamageItemEffect::applyTo(const entt::entity &Et,
                               entt::registry &Reg) const {
  Reg.get<HealthComp>(Et).reduce(Amount);
}

DismantleEffect::DismantleEffect(const ItemDatabase &DB,
                                 std::vector<DismantleResult> Results)
    : ItemDb(DB), Results(std::move(Results)) {}

std::shared_ptr<ItemEffect> DismantleEffect::clone() const {
  return std::make_shared<DismantleEffect>(*this);
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

bool DismantleEffect::canApplyTo(const entt::entity &Et,
                                 entt::registry &Reg) const {
  return Reg.all_of<InventoryComp>(Et);
}

void DismantleEffect::applyTo(const entt::entity &Et,
                              entt::registry &Reg) const {
  auto &Inv = Reg.get<InventoryComp>(Et);
  for (const auto &Result : Results) {
    Inv.Inv.addItem(ItemDb.createItem(Result.ItemId, Result.Amount));
  }
  // FIXME we need to make item entities and destroy the item here in the
  // registry (currently handled in UI)
}

} // namespace rogue