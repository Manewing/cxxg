#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/Item.h>
#include <rogue/ItemEffect.h>
#include <rogue/UI/Item.h>
#include <sstream>

namespace rogue::ui {

static constexpr auto EquipBaseColor = cxxg::types::RgbColor{130, 160, 210};

cxxg::types::TermColor getColorForItemType(ItemType Type) {
  if (Type & ItemType::Quest) {
    return cxxg::types::RgbColor{195, 196, 90};
  }
  if (Type & ItemType::EquipmentMask) {
    return EquipBaseColor;
  }
  if (Type & ItemType::Consumable) {
    return cxxg::types::RgbColor{112, 124, 219};
  }
  if (Type & ItemType::Crafting) {
    return cxxg::types::RgbColor{182, 186, 214};
  }
  return cxxg::types::Color::NONE;
}

namespace {

cxxg::types::TermColor
getColorForItemEffects(const std::vector<EffectInfo> &Effects,
                       CapabilityFlags Flags) {
  unsigned Count = 0;
  for (const auto &EffInfo : Effects) {
    if (EffInfo.Flags & Flags) {
      ++Count;
    }
  }
  if (Count <= 1) {
    return EquipBaseColor;
  }
  if (Count <= 2) {
    return cxxg::types::RgbColor{50, 150, 50};
  }
  if (Count <= 3) {
    return cxxg::types::RgbColor{185, 215, 50};
  }
  if (Count <= 4) {
    return cxxg::types::RgbColor{240, 125, 50};
  }
  return cxxg::types::RgbColor{240, 60, 180};
}

} // namespace

cxxg::types::TermColor getColorForItem(const Item &It) {
  const auto AllEffects = It.getAllEffects();
  if (It.getType() & ItemType::EquipmentMask && AllEffects.size() > 1) {
    return getColorForItemEffects(AllEffects, CapabilityFlags::Equipment);
  }
  return getColorForItemType(It.getType());
}

std::string getEffectDescription(const EffectInfo &EffInfo) {
  std::stringstream SS;

  const auto *Eff = EffInfo.Effect.get();
  if (const auto *HIE = dynamic_cast<const HealItemEffect *>(Eff)) {
    SS << "Heals " << HIE->getAmount() << " HP\n";
  }
  if (const auto *DIE = dynamic_cast<const DamageItemEffect *>(Eff)) {
    SS << "Deals " << DIE->getAmount() << " damage\n";
  }
  if (const auto *ABIE = dynamic_cast<const ApplyBuffItemEffectBase *>(Eff)) {
    (void)ABIE;
    SS << ABIE->getBuff().getDescription() << "\n";
  }
  if (const auto *MA = SetComponentEffect<MeleeAttackComp>::getOrNull(*Eff)) {
    if (MA->PhysDamage > 0) {
      SS << MA->PhysDamage << " melee phys. dmg.\n";
    }
    if (MA->MagicDamage > 0) {
      SS << MA->MagicDamage << " melee magic dmg.\n";
    }
  }
  if (const auto *RA = SetComponentEffect<RangedAttackComp>::getOrNull(*Eff)) {
    if (RA->PhysDamage > 0) {
      SS << RA->PhysDamage << " ranged phys. dmg.\n";
    }
    if (RA->MagicDamage > 0) {
      SS << RA->MagicDamage << " ranged magic dmg.\n";
    }
  }
  return SS.str();
}

std::string getCapabilityDescription(const std::vector<EffectInfo> &AllEffects,
                                     CapabilityFlags Flag) {
  std::stringstream SS;
  SS << Flag << ":\n";
  bool HasAny = false;
  for (const auto &EffInfo : AllEffects) {
    if ((EffInfo.Flags & Flag) != Flag) {
      continue;
    }
    HasAny = true;
    SS << getEffectDescription(EffInfo);
  }
  if (!HasAny) {
    return "";
  }
  SS << "\n";
  return SS.str();
}

std::string getItemEffectDescription(const Item &It) {
  std::stringstream SS;
  const auto AllEffects = It.getAllEffects();
  SS << getCapabilityDescription(
            AllEffects, (CapabilityFlags::Self | CapabilityFlags::UseOn))
     << getCapabilityDescription(
            AllEffects, (CapabilityFlags::Ranged | CapabilityFlags::UseOn))
     << getCapabilityDescription(
            AllEffects, (CapabilityFlags::Adjacent | CapabilityFlags::UseOn))
     << getCapabilityDescription(AllEffects, CapabilityFlags::EquipOn);
  return SS.str();
}

std::string getItemText(const Item &It) {
  std::stringstream SS;
  SS << "Type: " << It.getType() << "\n---\n"
     << getItemEffectDescription(It) << "---\n"
     << It.getDescription();
  return SS.str();
}

} // namespace rogue::ui