#include <rogue/Components/Stats.h>
#include <rogue/Item.h>

namespace rogue {

HealItemEffect::HealItemEffect(StatValue Amount) : Amount(Amount) {}

bool HealItemEffect::canUseOn(const entt::entity &Et,
                              entt::registry &Reg) const {
  return Reg.any_of<HealthComp>(Et);
}

int HealItemEffect::useOn(const entt::entity &Et, entt::registry &Reg,
                          int Num) const {
  assert(canUseOn(Et, Reg));
  Reg.get<HealthComp>(Et).restore(Amount * Num);
  return Num;
}

DamageItemEffect::DamageItemEffect(StatValue Amount) : Amount(Amount) {}

bool DamageItemEffect::canUseOn(const entt::entity &Et,
                                entt::registry &Reg) const {
  return Reg.any_of<HealthComp>(Et);
}

int DamageItemEffect::useOn(const entt::entity &Et, entt::registry &Reg,
                            int Num) const {
  Reg.get<HealthComp>(Et).reduce(Amount * Num);
  return Num;
}

ItemPrototype::ItemPrototype(int ItemId, std::string N, ItemType Type,
                             int MaxStatckSize,
                             std::vector<std::shared_ptr<ItemEffect>> Eff)
    : ItemId(ItemId), Name(std::move(N)), Type(Type),
      MaxStatckSize(MaxStatckSize), Effects(std::move(Eff)) {}

bool ItemPrototype::canUseOn(const entt::entity &Entity,
                             entt::registry &Reg) const {
  if (Effects.empty()) {
    return false;
  }
  bool CanUse = true;
  for (const auto &Effect : Effects) {
    CanUse = CanUse && Effect->canUseOn(Entity, Reg);
  }
  return CanUse;
}

void ItemPrototype::useOn(const entt::entity &Entity, entt::registry &Reg,
                          int Num) const {
  for (const auto &Effect : Effects) {
    Effect->useOn(Entity, Reg, Num);
  }
}

Item::Item(const ItemPrototype &Proto, int StackSize)
    : StackSize(StackSize), Proto(&Proto) {}

const ItemPrototype &Item::getProto() const { return *Proto; }

} // namespace rogue