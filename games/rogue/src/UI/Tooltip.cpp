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

std::string getItemTypeLabel(ItemType It) {
  std::stringstream Label;
  const char *Pred = "";
  if ((It & ItemType::Ring) != ItemType::None) {
    Label << Pred << "Ring";
    Pred = ", ";
  }
  if ((It & ItemType::Amulet) != ItemType::None) {
    Label << Pred << "Amulet";
    Pred = ", ";
  }
  if ((It & ItemType::Helmet) != ItemType::None) {
    Label << Pred << "Helmet";
    Pred = ", ";
  }
  if ((It & ItemType::ChestPlate) != ItemType::None) {
    Label << Pred << "Chest Plate";
    Pred = ", ";
  }
  if ((It & ItemType::Pants) != ItemType::None) {
    Label << Pred << "Pants";
    Pred = ", ";
  }
  if ((It & ItemType::Boots) != ItemType::None) {
    Label << Pred << "Boots";
    Pred = ", ";
  }
  if ((It & ItemType::Weapon) != ItemType::None) {
    Label << Pred << "Weapon";
    Pred = ", ";
  }
  if ((It & ItemType::OffHand) != ItemType::None) {
    Label << Pred << "Off Hand";
    Pred = ", ";
  }
  if ((It & ItemType::Generic) != ItemType::None) {
    Label << Pred << "Generic";
    Pred = ", ";
  }
  if ((It & ItemType::Consumable) != ItemType::None) {
    Label << Pred << "Consumable";
    Pred = ", ";
  }
  if ((It & ItemType::Quest) != ItemType::None) {
    Label << Pred << "Quest";
    Pred = ", ";
  }
  if ((It & ItemType::Crafting) != ItemType::None) {
    Label << Pred << "Crafting";
    Pred = ", ";
  }
  auto LabelStr = Label.str();
  if (LabelStr.empty()) {
    return "<unimp. ItemType>";
  }
  return LabelStr;
}

const char *getCapabilityFlag(CapabilityFlags Flags) {
  switch (Flags) {
  case CapabilityFlags::None:
    return "None";
  case CapabilityFlags::UseOn:
    return "Use";
  case CapabilityFlags::EquipOn:
    return "Equip";
  case CapabilityFlags::UnequipFrom:
    return "Unequip";
  case CapabilityFlags::Equipment:
    return "Equipment";
  default:
    break;
  }
  return "<unimp. CapabilityFlags>";
}

std::string getEffectDescription(const ItemPrototype::EffectInfo &EffInfo) {
  std::stringstream SS;
  SS << getCapabilityFlag(EffInfo.Flags) << ": ";
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