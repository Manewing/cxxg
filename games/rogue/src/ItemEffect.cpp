#include <rogue/Components/Items.h>
#include <rogue/ItemDatabase.h>
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

DismantleEffect::DismantleEffect(const ItemDatabase &DB,
                                 std::vector<DismantleResult> Results)
    : ItemDb(DB), Results(std::move(Results)) {}

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