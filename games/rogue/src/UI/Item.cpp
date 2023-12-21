#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/Item.h>
#include <rogue/ItemEffect.h>
#include <rogue/ItemPrototype.h>
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

std::string getCapabilityDescription(const ItemType &ItType,
                                     const std::vector<EffectInfo> &AllEffects,
                                     CapabilityFlags Flag) {
  std::stringstream SS;
  SS << Flag << ":\n";
  bool HasAny = false;
  for (const auto &EffInfo : AllEffects) {
    if ((EffInfo.Flags & Flag) != Flag) {
      continue;
    }
    HasAny = true;
    SS << "- " << EffInfo.Effect->getDescription() << "\n";
  }
  if (!HasAny) {
    return "";
  }
  if (!ItemPrototype::canApply(ItType, Flag)) {
    return "Crafting only: " + SS.str();
  }
  return SS.str();
}

std::string getItemEffectDescription(const Item &It) {
  std::stringstream SS;
  const auto AllEffects = It.getAllEffects();

  SS << getCapabilityDescription(
            It.getType(), AllEffects,
            (CapabilityFlags::Self | CapabilityFlags::UseOn))
     << getCapabilityDescription(
            It.getType(), AllEffects,
            (CapabilityFlags::Ranged | CapabilityFlags::UseOn))
     << getCapabilityDescription(
            It.getType(), AllEffects,
            (CapabilityFlags::Adjacent | CapabilityFlags::UseOn))
     << getCapabilityDescription(It.getType(), AllEffects,
                                 CapabilityFlags::EquipOn)
     << getCapabilityDescription(It.getType(), AllEffects,
                                 CapabilityFlags::Dismantle);
  return SS.str();
}

std::string getItemText(const Item &It) {
  std::stringstream SS;
  SS << "Type: " << It.getType() << "\n---\n";
  SS << getItemEffectDescription(It) << "---\n" << It.getDescription();
  return SS.str();
}

} // namespace rogue::ui