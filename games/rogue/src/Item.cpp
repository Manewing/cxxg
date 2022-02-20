#include "Item.h"
#include "Entity.h"

HealItemEffect::HealItemEffect(unsigned Amount) : Amount(Amount) {}

int HealItemEffect::apply(Entity &E, int Num) const {
  E.heal(Amount * Num);
  return Num;
}

DamageItemEffect::DamageItemEffect(unsigned Amount) : Amount(Amount) {}

int DamageItemEffect::apply(Entity &E, int Num) const {
  E.damage(Amount * Num);
  return Num;
}

ItemPrototype::ItemPrototype(int ItemId, std::string N, ItemType Type,
                             int MaxStatckSize,
                             std::vector<std::shared_ptr<ItemEffect>> Eff)
    : ItemId(ItemId), Name(std::move(N)), Type(Type),
      MaxStatckSize(MaxStatckSize), Effects(std::move(Eff)) {}

Item::Item(const ItemPrototype &Proto, int StackSize)
    : StackSize(StackSize), Proto(&Proto) {}

const ItemPrototype &Item::getProto() const { return *Proto; }