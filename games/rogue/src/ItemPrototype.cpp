#include <rogue/ItemEffect.h>
#include <rogue/ItemPrototype.h>

namespace rogue {

bool ItemPrototype::canApply(ItemType Type, CapabilityFlags Flags) {
  return
      // Equipment
      ((Flags & CapabilityFlags::Equipment) != CapabilityFlags::None &&
       (Type & ItemType::EquipmentMask) != ItemType::None) ||
      // Consumable
      ((Flags & CapabilityFlags::UseOn) != CapabilityFlags::None &&
       (Type & ItemType::Consumable) != ItemType::None);
}

ItemPrototype::ItemPrototype(int ItemId, std::string N, std::string D,
                             ItemType Type, int MaxStatckSize,
                             std::vector<EffectInfo> Eff)
    : ItemId(ItemId), Name(std::move(N)), Description(std::move(D)), Type(Type),
      MaxStackSize(MaxStatckSize), Effects(std::move(Eff)) {}

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

} // namespace rogue