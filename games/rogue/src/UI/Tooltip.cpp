#include <memory>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/ItemEffect.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/TextBox.h>
#include <rogue/UI/Tooltip.h>
#include <sstream>

namespace rogue::ui {

Tooltip::Tooltip(cxxg::types::Position Pos, cxxg::types::Size Size,
                 std::string Text, std::string Header)
    : Decorator(Pos, nullptr) {
  Comp = std::make_shared<TextBox>(Pos, Size, Text);
  Comp = std::make_shared<Frame>(Comp, Pos, Size, Header);
}

bool Tooltip::handleInput(int Char) {
  if (Char == Controls::Info.Char) {
    return false;
  }
  return Decorator::handleInput(Char);
}

namespace {

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

} // namespace

ItemTooltip::ItemTooltip(cxxg::types::Position Pos, cxxg::types::Size Size,
                         const Item &It, bool Equipped)
    : Tooltip(Pos, Size, getItemText(It),
              (Equipped ? "Equip: " : "") + It.getName()) {}

} // namespace rogue::ui