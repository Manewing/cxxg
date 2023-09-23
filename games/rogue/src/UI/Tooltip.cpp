#include <memory>
#include <rogue/Components/Buffs.h>
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
  if (const auto *HIE = dynamic_cast<HealItemEffect *>(EffInfo.Effect.get())) {
    SS << "Heals " << HIE->getAmount() << " HP\n";
  }
  if (const auto *DIE =
          dynamic_cast<DamageItemEffect *>(EffInfo.Effect.get())) {
    SS << "Deals " << DIE->getAmount() << " damage\n";
  }
  if (const auto *ABIE =
          dynamic_cast<ApplyBuffItemEffectBase *>(EffInfo.Effect.get())) {
    (void)ABIE;
    SS << ABIE->getBuff().getDescription() << "\n";
  }
  return SS.str();
}

std::string getCapabilityDescription(const std::vector<EffectInfo> &AllEffects,
                                     CapabilityFlags Flag) {
  std::stringstream SS;
  SS << getCapabilityFlagLabel(Flag) << ":\n";
  bool HasAny = false;
  for (const auto &EffInfo : AllEffects) {
    if ((EffInfo.Flags & Flag) == CapabilityFlags::None) {
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
  SS << getCapabilityDescription(AllEffects, CapabilityFlags::UseOn)
     << getCapabilityDescription(AllEffects, CapabilityFlags::Equipment);
  return SS.str();
}

std::string getItemText(const Item &It) {
  std::stringstream SS;
  SS << "Type: " << getItemTypeLabel(It.getType()) << "\n---\n"
     << getItemEffectDescription(It) << "---\n"
     << It.getDescription();
  return SS.str();
}

} // namespace

ItemTooltip::ItemTooltip(cxxg::types::Position Pos, cxxg::types::Size Size,
                         const Item &It)
    : Tooltip(Pos, Size, getItemText(It), It.getName()) {}

} // namespace rogue::ui