#include <rogue/ItemEffect.h>
#include <rogue/ItemPrototype.h>
#include <sstream>

namespace rogue {

bool ItemPrototype::canApply(ItemType Type, CapabilityFlags Flags) {
  // FIXME this should be moved to ItemType
  return
      // Equipment
      ((Flags & (CapabilityFlags::Equipment | CapabilityFlags::Skill)) &&
       (Type & ItemType::EquipmentMask)) ||
      // Consumable
      ((Flags & CapabilityFlags::UseOn) && (Type & ItemType::Consumable))
      // Dismantle
      || ((Flags & CapabilityFlags::Dismantle));
}

ItemPrototype::ItemPrototype(ItemProtoId ItemId, std::string N, std::string D,
                             ItemType Type, int MaxStatckSize,
                             std::vector<EffectInfo> Eff)
    : ItemId(ItemId), Name(std::move(N)), Description(std::move(D)), Type(Type),
      MaxStackSize(MaxStatckSize), Effects(std::move(Eff)) {
  if (MaxStackSize <= 0) {
    throw std::invalid_argument("Max stack size must be greater than 0");
  }
}

EffectAttributes ItemPrototype::getAttributes() const {
  EffectAttributes Attrs;
  for (const auto &Info : Effects) {
    Attrs.addFrom(Info.Attributes);
  }
  return Attrs;
}

EffectAttributes ItemPrototype::getAttributes(CapabilityFlags Flags) const {
  EffectAttributes Attrs;
  for (const auto &Info : Effects) {
    if (Info.Attributes.Flags.is(Flags)) {
      Attrs.addFrom(Info.Attributes);
    }
  }
  return Attrs;
}

bool ItemPrototype::hasEffect(CapabilityFlags Flags, bool AllowNull,
                              bool AllowRemove) const {
  for (const auto &Info : Effects) {
    if (!(Info.Attributes.Flags.is(Flags))) {
      continue;
    }
    const bool IsNullEffect =
        dynamic_cast<const NullEffect *>(Info.Effect.get());
    if (IsNullEffect && !AllowNull) {
      continue;
    }
    const bool IsRemoveEffect =
        dynamic_cast<const RemoveEffectBase *>(Info.Effect.get());
    if (IsRemoveEffect && !AllowRemove) {
      continue;
    }
    return true;
  }
  return false;
}

CapabilityFlags ItemPrototype::getCapabilityFlags() const {
  return getAttributes().Flags;
}

bool ItemPrototype::checkCapabilityFlags(CapabilityFlags Flags) const {
  return canApply(Type, Flags) && bool(getCapabilityFlags().is(Flags));
}

bool ItemPrototype::canApplyTo(const entt::entity &SrcEt,
                               const entt::entity &DstEt, entt::registry &Reg,
                               CapabilityFlags Flags) const {
  if (!checkCapabilityFlags(Flags) ||
      !getAttributes(Flags).checkCosts(SrcEt, Reg)) {
    return false;
  }
  bool CanApply = true;
  for (const auto &Info : Effects) {
    if (Info.Attributes.Flags.is(Flags)) {
      CanApply = CanApply && Info.canApplyTo(SrcEt, DstEt, Reg, Flags);
    }
  }
  return CanApply;
}

void ItemPrototype::applyTo(const entt::entity &SrcEt,
                            const entt::entity &DstEt, entt::registry &Reg,
                            CapabilityFlags Flags) const {
  if (!checkCapabilityFlags(Flags) ||
      !getAttributes(Flags).checkCosts(SrcEt, Reg)) {
    std::stringstream SS;
    SS << "Can't apply item '" << Name << "' to entity with flags " << Flags
       << " (item flags: " << getCapabilityFlags() << ", item type: " << Type
       << ", attrs: " << getAttributes() << ")";
    throw std::runtime_error(SS.str());
  }
  for (const auto &Info : Effects) {
    if (Info.Attributes.Flags.is(Flags)) {
      Info.applyTo(SrcEt, DstEt, Reg, Flags);
    }
  }
}

bool ItemPrototype::canRemoveFrom(const entt::entity &SrcEt,
                                  const entt::entity &DstEt,
                                  entt::registry &Reg,
                                  CapabilityFlags Flags) const {
  if (!checkCapabilityFlags(Flags)) {
    return false;
  }
  bool CanRemove = true;
  for (const auto &Info : Effects) {
    if (Info.Attributes.Flags.is(Flags)) {
      CanRemove = CanRemove && Info.canRemoveFrom(SrcEt, DstEt, Reg, Flags);
    }
  }
  return CanRemove;
}

void ItemPrototype::removeFrom(const entt::entity &SrcEt,
                               const entt::entity &DstEt, entt::registry &Reg,
                               CapabilityFlags Flags) const {
  if (!checkCapabilityFlags(Flags)) {
    std::stringstream SS;
    SS << "Can't remove item '" << Name << "' from entity with flags " << Flags
       << " (item flags: " << getCapabilityFlags() << ", item type: " << Type
       << ", attrs: " << getAttributes() << ")";
    throw std::runtime_error(SS.str());
  }
  for (const auto &Info : Effects) {
    if (Info.Attributes.Flags.is(Flags)) {
      Info.removeFrom(SrcEt, DstEt, Reg, Flags);
    }
  }
}

} // namespace rogue