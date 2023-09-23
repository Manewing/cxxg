#include <rogue/Item.h>
#include <rogue/ItemEffect.h>
#include <rogue/ItemPrototype.h>

namespace rogue {
Item::Item(const ItemPrototype &Proto, int StackSize,
           const std::shared_ptr<ItemPrototype> &Spec)
    : StackSize(StackSize), Proto(&Proto), Specialization(Spec) {}

std::string Item::getName() const {
  // TODO make name depend on quality etc
  return getProto().Name;
}

std::string Item::getDescription() const {
  // TODO make description depend on quality etc
  return getProto().Description;
}

ItemType Item::getType() const {
  if (Specialization) {
    return getProto().Type | Specialization->Type;
  }
  return getProto().Type;
}

int Item::getMaxStackSize() const { return getProto().MaxStackSize; }

std::vector<EffectInfo> Item::getAllEffects() const {
  std::vector<EffectInfo> AllEffects;
  auto NumEffects = getProto().Effects.size();
  NumEffects += Specialization ? Specialization->Effects.size() : 0;
  AllEffects.reserve(NumEffects);

  for (const auto &Info : getProto().Effects) {
    AllEffects.push_back(Info);
  }
  if (Specialization) {
    for (const auto &Info : Specialization->Effects) {
      AllEffects.push_back(Info);
    }
  }
  return AllEffects;
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