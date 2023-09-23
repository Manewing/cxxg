#include <memory>
#include <rogue/Components/Buffs.h>
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

namespace {


std::string getEffectDescription(const ItemPrototype::EffectInfo &EffInfo) {
  std::stringstream SS;
  SS << getCapabilityFlagLabel(EffInfo.Flags) << ": ";
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

std::string getItemEffectDescription(const Item &It) {
  std::stringstream SS;
  for (const auto &EffInfo : It.getAllEffects()) {
    SS << getEffectDescription(EffInfo);
  }
  return SS.str();
}

std::string getItemText(const Item &It) {
  std::stringstream SS;
  SS << "Type: " << getItemTypeLabel(It.getType()) << "\n"
     << getItemEffectDescription(It) << "\n"
     << "---\n"
     << It.getDescription();
  return SS.str();
}

} // namespace

ItemTooltip::ItemTooltip(cxxg::types::Position Pos, cxxg::types::Size Size,
                         const Item &It)
    : Tooltip(Pos, Size, getItemText(It), It.getName()) {}

} // namespace rogue::ui