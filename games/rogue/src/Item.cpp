#include <rogue/Components/Stats.h>
#include <rogue/Item.h>

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

bool ItemPrototype::canApply(ItemType Type, CapabilityFlags Flags) {
  return
      // Equipment
      ((Flags & CapabilityFlags::Equipment) != CapabilityFlags::None &&
       (Type & ItemType::EquipmentMask) != ItemType::None) ||
      // Consumable
      ((Flags & CapabilityFlags::UseOn) != CapabilityFlags::None &&
       (Type & ItemType::Consumable) != ItemType::None);
}

ItemPrototype::ItemPrototype(int ItemId, std::string N, ItemType Type,
                             int MaxStatckSize, std::vector<EffectInfo> Eff)
    : ItemId(ItemId), Name(std::move(N)), Type(Type),
      MaxStatckSize(MaxStatckSize), Effects(std::move(Eff)) {}

bool ItemPrototype::canApplyTo(const entt::entity &Entity, entt::registry &Reg,
                               CapabilityFlags Flags) const {
  if (!canApply(Type, Flags)) {
    return false;
  }
  bool CanApply = true;
  for (const auto &Info : Effects) {
    if ((Info.Flags & Flags) != CapabilityFlags::None) {
      CanApply = CanApply && Info.Effect->canApplyTo(Entity, Reg);
    }
  }
  return CanApply;
}

void ItemPrototype::applyTo(const entt::entity &Entity, entt::registry &Reg,
                            CapabilityFlags Flags) const {
  if (!canApply(Type, Flags)) {
    return;
  }
  for (const auto &Info : Effects) {
    if ((Info.Flags & Flags) != CapabilityFlags::None) {
      Info.Effect->applyTo(Entity, Reg);
    }
  }
}

bool ItemPrototype::canRemoveFrom(const entt::entity &Entity,
                                  entt::registry &Reg,
                                  CapabilityFlags Flags) const {
  if (!canApply(Type, Flags)) {
    return false;
  }
  bool CanRemove = true;
  for (const auto &Info : Effects) {
    if ((Info.Flags & Flags) != CapabilityFlags::None) {
      CanRemove = CanRemove && Info.Effect->canRemoveFrom(Entity, Reg);
    }
  }
  return CanRemove;
}

void ItemPrototype::removeFrom(const entt::entity &Entity, entt::registry &Reg,
                               CapabilityFlags Flags) const {
  if (!canApply(Type, Flags)) {
    return;
  }
  for (const auto &Info : Effects) {
    if ((Info.Flags & Flags) != CapabilityFlags::None) {
      Info.Effect->removeFrom(Entity, Reg);
    }
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

bool Item::canApplyTo(const entt::entity &Entity, entt::registry &Reg,
                      CapabilityFlags Flags) const {
  bool SpecCanUseOn =
      !Specialization || Specialization->canApplyTo(Entity, Reg, Flags);
  return getProto().canApplyTo(Entity, Reg, Flags) && SpecCanUseOn;
}

void Item::applyTo(const entt::entity &Entity, entt::registry &Reg,
                   CapabilityFlags Flags) const {
  if (Specialization) {
    Specialization->applyTo(Entity, Reg, Flags);
  }
  getProto().applyTo(Entity, Reg, Flags);
}

bool Item::canRemoveFrom(const entt::entity &Entity, entt::registry &Reg,
                         CapabilityFlags Flags) const {
  bool SpecCanUnequipFrom =
      !Specialization || Specialization->canRemoveFrom(Entity, Reg, Flags);
  return getProto().canRemoveFrom(Entity, Reg, Flags) && SpecCanUnequipFrom;
}

void Item::removeFrom(const entt::entity &Entity, entt::registry &Reg,
                      CapabilityFlags Flags) const {
  if (Specialization) {
    Specialization->removeFrom(Entity, Reg, Flags);
  }
  getProto().removeFrom(Entity, Reg, Flags);
}

const ItemPrototype &Item::getProto() const { return *Proto; }

EquipmentSlot &Equipment::getSlot(ItemType It) {
  return const_cast<EquipmentSlot &>(
      static_cast<const Equipment *>(this)->getSlot(It));
}

const EquipmentSlot &Equipment::getSlot(ItemType It) const {
  switch (It & ItemType::EquipmentMask) {
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
  return OffHand;
}

bool Equipment::isEquipped(ItemType Type) const {
  if ((Type & ItemType::EquipmentMask) == ItemType::None) {
    return false;
  }
  return getSlot(Type).It != std::nullopt;
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