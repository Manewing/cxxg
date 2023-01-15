#include <rogue/Components/Stats.h>
#include <rogue/Item.h>

namespace rogue {

HealItemEffect::HealItemEffect(StatValue Amount) : Amount(Amount) {}

bool HealItemEffect::canUseOn(const entt::entity &Et,
                              entt::registry &Reg) const {
  return Reg.any_of<HealthComp>(Et);
}

void HealItemEffect::useOn(const entt::entity &Et, entt::registry &Reg) const {
  assert(canUseOn(Et, Reg));
  Reg.get<HealthComp>(Et).restore(Amount);
}

DamageItemEffect::DamageItemEffect(StatValue Amount) : Amount(Amount) {}

bool DamageItemEffect::canUseOn(const entt::entity &Et,
                                entt::registry &Reg) const {
  return Reg.any_of<HealthComp>(Et);
}

void DamageItemEffect::useOn(const entt::entity &Et,
                             entt::registry &Reg) const {
  Reg.get<HealthComp>(Et).reduce(Amount);
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

void ItemPrototype::useOn(const entt::entity &Entity,
                          entt::registry &Reg) const {
  for (const auto &Effect : Effects) {
    Effect->useOn(Entity, Reg);
  }
}

bool ItemPrototype::canEquipOn(const entt::entity &Entity,
                               entt::registry &Reg) const {
  if (Effects.empty()) {
    return false;
  }
  bool CanEquip = true;
  for (const auto &Effect : Effects) {
    CanEquip = CanEquip && Effect->canEquipOn(Entity, Reg);
  }
  return CanEquip;
}

void ItemPrototype::equipOn(const entt::entity &Entity,
                            entt::registry &Reg) const {
  for (const auto &Effect : Effects) {
    Effect->equipOn(Entity, Reg);
  }
}

bool ItemPrototype::canUnequipFrom(const entt::entity &Entity,
                                   entt::registry &Reg) const {
  if (Effects.empty()) {
    return false;
  }
  bool CanUnequip = true;
  for (const auto &Effect : Effects) {
    CanUnequip = CanUnequip && Effect->canUnequipFrom(Entity, Reg);
  }
  return CanUnequip;
}

void ItemPrototype::unequipFrom(const entt::entity &Entity,
                                entt::registry &Reg) const {
  for (const auto &Effect : Effects) {
    Effect->unequipFrom(Entity, Reg);
  }
}

Item::Item(const ItemPrototype &Proto, int StackSize,
           const std::shared_ptr<ItemPrototype> &Spec)
    : StackSize(StackSize), Proto(&Proto), Specialization(Spec) {}

std::string Item::getName() const {
  // TODO make name depend on quality etc
  return getProto().Name;
}

ItemType Item::getType() const {
  if (Specialization) {
    return getProto().Type | Specialization->Type;
  }
  return getProto().Type;
}

bool Item::isSameKind(const Item &Other) const {
  return Proto == Other.Proto && Specialization == Other.Specialization;
}

bool Item::canUseOn(const entt::entity &Entity, entt::registry &Reg) const {
  bool SpecCanUseOn = Specialization && Specialization->canUseOn(Entity, Reg);
  return getProto().canUseOn(Entity, Reg) && SpecCanUseOn;
}

void Item::useOn(const entt::entity &Entity, entt::registry &Reg) const {
  if (Specialization) {
    Specialization->useOn(Entity, Reg);
  }
  getProto().useOn(Entity, Reg);
}

bool Item::canEquipOn(const entt::entity &Entity, entt::registry &Reg) const {
  bool SpecCanEquipOn =
      !Specialization || Specialization->canEquipOn(Entity, Reg);
  return getProto().canEquipOn(Entity, Reg) && SpecCanEquipOn;
}

void Item::equipOn(const entt::entity &Entity, entt::registry &Reg) const {
  if (Specialization) {
    Specialization->equipOn(Entity, Reg);
  }
  getProto().equipOn(Entity, Reg);
}

bool Item::canUnequipFrom(const entt::entity &Entity,
                          entt::registry &Reg) const {
  bool SpecCanUnequipFrom =
      !Specialization || Specialization->canUnequipFrom(Entity, Reg);
  return getProto().canUnequipFrom(Entity, Reg) && SpecCanUnequipFrom;
}

void Item::unequipFrom(const entt::entity &Entity, entt::registry &Reg) const {
  if (Specialization) {
    Specialization->unequipFrom(Entity, Reg);
  }
  getProto().unequipFrom(Entity, Reg);
}

const ItemPrototype &Item::getProto() const { return *Proto; }

EquipmentSlot &Equipment::getSlot(ItemType It) {
  return const_cast<EquipmentSlot &>(
      static_cast<const Equipment *>(this)->getSlot(It));
}

const EquipmentSlot &Equipment::getSlot(ItemType It) const {
  switch (It) {
  case ItemType::Ring:
    return Ring;
  case ItemType::Amulet:
    return Amulet;
  case ItemType::Helmet:
    return Helmet;
  case ItemType::ChestPlate:
    return ChestPlate;
  case ItemType::Pants:
    return Pants;
  case ItemType::Boots:
    return Boots;
  case ItemType::Weapon:
    return Weapon;
  case ItemType::OffHand:
    return OffHand;
  default:
    break;
  }
  assert(false);
  return Ring;
}

bool Equipment::canEquip(ItemType Type) const {
  if ((Type & ItemType::EquipmentMask) == ItemType::None) {
    return false;
  }
  return getSlot(Type).It == std::nullopt;
}

void Equipment::equip(Item Item) {
  auto Type = Item.getType();
  auto &ES = getSlot(Type);
  ES.It = std::move(Item);
}

Item Equipment::unequip(ItemType Type) {
  auto &ES = getSlot(Type);
  Item It = std::move(*ES.It);
  ES.It = std::nullopt;
  return It;
}

} // namespace rogue