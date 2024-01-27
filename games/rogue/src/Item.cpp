#include <rogue/Components/Buffs.h>
#include <rogue/Item.h>
#include <rogue/ItemEffect.h>
#include <rogue/ItemPrototype.h>

namespace rogue {

Item::Item(const ItemPrototype &Proto, int StackSize,
           const std::shared_ptr<const ItemPrototype> &Spec, bool SpecOverrides)
    : StackSize(StackSize), Proto(&Proto), Specialization(Spec),
      SpecOverrides(SpecOverrides) {}

namespace {

std::string getQualifierNameForHash(std::size_t Hash) {
  static constexpr std::array Runes = {"f", "u", "t", "o", "r", "k", "o",
                                       "n", "i", "s", "x", "v", "y", "z"};
  std::string Name;
  Name.reserve(6);
  for (std::size_t I = 0; I < 5; ++I) {
    Name += Runes[Hash % Runes.size()];
    Hash /= Runes.size();
  }
  Name[0] = std::toupper(Name[0]);
  Name += ' ';
  return Name;
}

std::string getQualifierNameForItem(const Item &It, CapabilityFlags Flags) {
  // Check for stats buff effect and return name based strongest
  // stat point boost
  std::size_t Hash = 0;
  for (const auto &ItEff : It.getAllEffects()) {
    if (!(ItEff.Attributes.Flags.is(Flags))) {
      continue;
    }
    if (dynamic_cast<const NullEffect *>(ItEff.Effect.get())) {
      continue;
    }
    Hash ^= std::hash<std::string>{}(ItEff.Effect->getDescription());
  }
  if (Hash != 0) {
    return getQualifierNameForHash(Hash);
  }
  return "";
}

} // namespace

int Item::getId() const { return getProto().ItemId; }

const std::string &Item::getName() const {
  if (!Specialization || !SpecOverrides) {
    return getProto().Name;
  } else {
    return Specialization->Name;
  }
}

std::string Item::getQualifierName() const {
  auto Name = getProto().Name;
  if (Specialization && SpecOverrides) {
    Name = Specialization->Name;
  }
  if (getType() & ItemType::EquipmentMask) {
    return getQualifierNameForItem(*this, CapabilityFlags::Equipment) + Name;
  }
  if (getType().is(ItemType::CraftingBase | ItemType::Consumable)) {
    return getQualifierNameForItem(*this, CapabilityFlags::UseOn) + Name;
  }
  return Name;
}

std::string Item::getDescription() const {
  if (Specialization && SpecOverrides) {
    return Specialization->Description;
  }
  return getProto().Description;
}

ItemType Item::getType() const {
  if (Specialization) {
    if (SpecOverrides) {
      return Specialization->Type;
    }
    return getProto().Type | Specialization->Type;
  }
  return getProto().Type;
}

int Item::getMaxStackSize() const {
  if (Specialization && SpecOverrides) {
    return Specialization->MaxStackSize;
  }
  return getProto().MaxStackSize;
}

std::vector<EffectInfo> Item::getAllEffects() const {
  if (Specialization && SpecOverrides) {
    return Specialization->Effects;
  }
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

bool Item::hasEffect(CapabilityFlags Flags, bool AllowNull,
                     bool AllowRemove) const {
  bool HasEffect = false;
  if (Specialization) {
    HasEffect = Specialization->hasEffect(Flags, AllowNull, AllowRemove);
    if (SpecOverrides) {
      return HasEffect;
    }
  }
  return HasEffect || getProto().hasEffect(Flags, AllowNull, AllowRemove);
}

CapabilityFlags Item::getCapabilityFlags() const {
  if (Specialization && SpecOverrides) {
    return Specialization->getCapabilityFlags();
  }

  auto Flags = getProto().getCapabilityFlags();
  if (Specialization) {
    Flags = Flags | Specialization->getCapabilityFlags();
  }
  return Flags;
}

bool Item::isSameKind(const Item &Other) const {
  return Proto == Other.Proto && Specialization == Other.Specialization;
}

bool Item::canApplyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                      entt::registry &Reg, CapabilityFlags Flags) const {
  if (Specialization && SpecOverrides) {
    return Specialization->canApplyTo(SrcEt, DstEt, Reg, Flags);
  }

  // Compute overall attributes to check overall costs
  bool SpecCanUseOn = false;
  auto Attrs = getProto().getAttributes(Flags);
  if (Specialization) {
    SpecCanUseOn = Specialization->canApplyTo(SrcEt, DstEt, Reg, Flags);
    if (SpecCanUseOn) {
      Attrs.addFrom(Specialization->getAttributes(Flags));
    }
  }
  if (!Attrs.checkCosts(SrcEt, Reg)) {
    return false;
  }

  return getProto().canApplyTo(SrcEt, DstEt, Reg, Flags) || SpecCanUseOn;
}

void Item::applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                   entt::registry &Reg, CapabilityFlags Flags) const {
  if (Specialization && Specialization->canApplyTo(SrcEt, DstEt, Reg, Flags)) {
    Specialization->applyTo(SrcEt, DstEt, Reg, Flags);
    if (SpecOverrides) {
      return;
    }
  }
  if (getProto().canApplyTo(SrcEt, DstEt, Reg, Flags)) {
    getProto().applyTo(SrcEt, DstEt, Reg, Flags);
  }
}

bool Item::canRemoveFrom(const entt::entity &SrcEt, const entt::entity &DstEt,
                         entt::registry &Reg, CapabilityFlags Flags) const {
  if (Specialization && SpecOverrides) {
    return Specialization->canRemoveFrom(SrcEt, DstEt, Reg, Flags);
  }
  bool SpecCanUnequipFrom =
      Specialization && Specialization->canRemoveFrom(SrcEt, DstEt, Reg, Flags);
  return getProto().canRemoveFrom(SrcEt, DstEt, Reg, Flags) ||
         SpecCanUnequipFrom;
}

void Item::removeFrom(const entt::entity &SrcEt, const entt::entity &DstEt,
                      entt::registry &Reg, CapabilityFlags Flags) const {
  if (Specialization &&
      Specialization->canRemoveFrom(SrcEt, DstEt, Reg, Flags)) {
    Specialization->removeFrom(SrcEt, DstEt, Reg, Flags);
    if (SpecOverrides) {
      return;
    }
  }
  if (getProto().canRemoveFrom(SrcEt, DstEt, Reg, Flags)) {
    getProto().removeFrom(SrcEt, DstEt, Reg, Flags);
  }
}

const ItemPrototype &Item::getProto() const { return *Proto; }

const std::shared_ptr<const ItemPrototype> &Item::getSpecialization() const {
  return Specialization;
}

} // namespace rogue