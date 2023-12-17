#include <rogue/Components/Buffs.h>
#include <rogue/Item.h>
#include <rogue/ItemEffect.h>
#include <rogue/ItemPrototype.h>

namespace rogue {

Item::Item(const ItemPrototype &Proto, int StackSize,
           const std::shared_ptr<ItemPrototype> &Spec)
    : StackSize(StackSize), Proto(&Proto), Specialization(Spec) {}

namespace {

std::string getQualifierName(const StatPoints &P) {
  std::string Prefix = "+" + std::to_string(P.sum()) + " ";
  if (P.Str == P.Dex && P.Str == P.Int && P.Str == P.Vit) {
    return Prefix + "Bal. ";
  }
  if (P.Str >= P.Dex && P.Str >= P.Int && P.Str >= P.Vit) {
    return Prefix + "Str. ";
  }
  if (P.Dex >= P.Str && P.Dex >= P.Int && P.Dex >= P.Vit) {
    return Prefix + "Fast ";
  }
  if (P.Int >= P.Str && P.Int >= P.Dex && P.Int >= P.Vit) {
    return Prefix + "Wise ";
  }
  if (P.Vit >= P.Str && P.Vit >= P.Dex && P.Vit >= P.Int) {
    return Prefix + "Tgh. ";
  }
  return Prefix;
}

std::string getQualifierName(const Item &It) {
  // Check for stats buff effect and return name based strongest
  // stat point boost
  for (const auto &ItEff : It.getAllEffects()) {
    if ((ItEff.Flags & CapabilityFlags::Equipment) == CapabilityFlags::None) {
      continue;
    }
    const auto *StatEff =
        dynamic_cast<const ApplyBuffItemEffectBase *>(ItEff.Effect.get());
    if (!StatEff) {
      continue;
    }
    const auto *StatBuff =
        dynamic_cast<const StatsBuffComp *>(&StatEff->getBuff());
    if (!StatBuff) {
      continue;
    }
    return getQualifierName(StatBuff->Bonus);
  }
  return "";
}

} // namespace

int Item::getId() const { return getProto().ItemId; }

std::string Item::getName() const {
  if (getType() & ItemType::EquipmentMask) {
    return getQualifierName(*this) + getProto().Name;
  }
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

CapabilityFlags Item::getCapabilityFlags() const {
  auto Flags = getProto().getCapabilityFlags();
  if (Specialization) {
    Flags = Flags | Specialization->getCapabilityFlags();
  }
  return Flags;
}

bool Item::isSameKind(const Item &Other) const {
  return Proto == Other.Proto && Specialization == Other.Specialization;
}

bool Item::canApplyTo(const entt::entity &Entity, entt::registry &Reg,
                      CapabilityFlags Flags) const {
  bool SpecCanUseOn =
      Specialization && Specialization->canApplyTo(Entity, Reg, Flags);
  return getProto().canApplyTo(Entity, Reg, Flags) || SpecCanUseOn;
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
      Specialization && Specialization->canRemoveFrom(Entity, Reg, Flags);
  return getProto().canRemoveFrom(Entity, Reg, Flags) || SpecCanUnequipFrom;
}

void Item::removeFrom(const entt::entity &Entity, entt::registry &Reg,
                      CapabilityFlags Flags) const {
  if (Specialization) {
    Specialization->removeFrom(Entity, Reg, Flags);
  }
  getProto().removeFrom(Entity, Reg, Flags);
}

const ItemPrototype &Item::getProto() const { return *Proto; }

} // namespace rogue