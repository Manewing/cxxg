#include <rogue/ItemEffect.h>
#include <rogue/ItemPrototype.h>
#include <sstream>

namespace rogue {

bool ItemPrototype::canApply(ItemType Type, CapabilityFlags Flags) {
  // FIXME this should be moved to ItemType
  return
      // Equipment
      ((Flags & CapabilityFlags::Equipment) != CapabilityFlags::None &&
       (Type & ItemType::EquipmentMask) != ItemType::None) ||
      // Consumable
      ((Flags & CapabilityFlags::UseOn) != CapabilityFlags::None &&
       (Type & ItemType::Consumable) != ItemType::None)
      // Dismantle
      || ((Flags & CapabilityFlags::Dismantle) != CapabilityFlags::None);
}

ItemPrototype::ItemPrototype(int ItemId, std::string N, std::string D,
                             ItemType Type, int MaxStatckSize,
                             std::vector<EffectInfo> Eff)
    : ItemId(ItemId), Name(std::move(N)), Description(std::move(D)), Type(Type),
      MaxStackSize(MaxStatckSize), Effects(std::move(Eff)) {}

CapabilityFlags ItemPrototype::getCapabilityFlags() const {
  CapabilityFlags Flags = CapabilityFlags::None;
  for (const auto &Info : Effects) {
    Flags = Flags | Info.Flags;
  }
  return Flags;
}

bool ItemPrototype::checkCapabilityFlags(CapabilityFlags Flags) const {
  return canApply(Type, Flags) && (Flags & getCapabilityFlags()) != CapabilityFlags::None;
}

bool ItemPrototype::canApplyTo(const entt::entity &Entity, entt::registry &Reg,
                               CapabilityFlags Flags) const {
  if (!checkCapabilityFlags(Flags)) {
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
  if (!checkCapabilityFlags(Flags)) {
    std::stringstream SS;
    SS << "Can't apply item '" << Name << "' to entity with flags "
       << getCapabilityFlagLabel(Flags)
       << " (item flags: " << getCapabilityFlagLabel(getCapabilityFlags())
       << ", item type: " << getItemTypeLabel(Type) << ")";
    throw std::runtime_error(SS.str());
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
  if (!checkCapabilityFlags(Flags)) {
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
  if (!checkCapabilityFlags(Flags)) {
    std::stringstream SS;
    SS << "Can't remove item '" << Name << "' from entity with flags "
       << getCapabilityFlagLabel(Flags)
       << " (item flags: " << getCapabilityFlagLabel(getCapabilityFlags())
       << ", item type: " << getItemTypeLabel(Type) << ")";
    throw std::runtime_error(SS.str());
  }
  for (const auto &Info : Effects) {
    if ((Info.Flags & Flags) != CapabilityFlags::None) {
      Info.Effect->removeFrom(Entity, Reg);
    }
  }
}

} // namespace rogue