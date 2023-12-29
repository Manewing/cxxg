#include <map>
#include <rogue/ItemType.h>
#include <sstream>
#include <stdexcept>

namespace rogue {

std::optional<ItemType> ItemType::parseString(const std::string &Type) {
  static const std::map<std::string, ItemType> ItemTypes = {
      {"none", ItemType::None}, // Keep top
      {"ring", ItemType::Ring}, //
      {"amulet", ItemType::Amulet},
      {"helmet", ItemType::Helmet},
      {"chest_plate", ItemType::ChestPlate},
      {"pants", ItemType::Pants},
      {"boots", ItemType::Boots},
      {"weapon", ItemType::Weapon},
      {"shield", ItemType::Shield},
      {"ranged", ItemType::Ranged},
      {"crafting_base", ItemType::CraftingBase},
      {"consumable", ItemType::Consumable},
      {"quest", ItemType::Quest},
      {"crafting", ItemType::Crafting},
  };
  if (const auto It = ItemTypes.find(Type); It != ItemTypes.end()) {
    return It->second;
  }
  return std::nullopt;
}

ItemType ItemType::fromString(const std::string &Type) {
  if (const auto It = parseString(Type); It.has_value()) {
    return It.value();
  }
  throw std::out_of_range("Unknown ItemType: " + Type);
}

std::string ItemType::str() const {
  std::stringstream Label;
  const char *Pred = "";
  if (Value & ItemType::Ring) {
    Label << Pred << "Ring";
    Pred = ", ";
  }
  if (Value & ItemType::Amulet) {
    Label << Pred << "Amulet";
    Pred = ", ";
  }
  if (Value & ItemType::Helmet) {
    Label << Pred << "Helmet";
    Pred = ", ";
  }
  if (Value & ItemType::ChestPlate) {
    Label << Pred << "Chest Plate";
    Pred = ", ";
  }
  if (Value & ItemType::Pants) {
    Label << Pred << "Pants";
    Pred = ", ";
  }
  if (Value & ItemType::Boots) {
    Label << Pred << "Boots";
    Pred = ", ";
  }
  if (Value & ItemType::Weapon) {
    Label << Pred << "Weapon";
    Pred = ", ";
  }
  if (Value & ItemType::Shield) {
    Label << Pred << "Shield";
    Pred = ", ";
  }
  if (Value & ItemType::Ranged) {
    Label << Pred << "Ranged";
    Pred = ", ";
  }
  if (Value & ItemType::CraftingBase) {
    Label << Pred << "Crafting Base";
    Pred = ", ";
  }
  if (Value & ItemType::Consumable) {
    Label << Pred << "Consumable";
    Pred = ", ";
  }
  if (Value & ItemType::Quest) {
    Label << Pred << "Quest";
    Pred = ", ";
  }
  if (Value & ItemType::Crafting) {
    Label << Pred << "Crafting";
    Pred = ", ";
  }
  auto LabelStr = Label.str();
  if (LabelStr.empty()) {
    return "<unimp. ValueemType>";
  }
  return LabelStr;
}

std::ostream &operator<<(std::ostream &Out, const ItemType &Type) {
  Out << Type.str();
  return Out;
}
std::optional<CapabilityFlags>
CapabilityFlags::parseString(const std::string &Str) {
  if (Str == "use_on") {
    return CapabilityFlags::UseOn | CapabilityFlags::Self;
  }
  if (Str == "equipment") {
    return CapabilityFlags::Equipment;
  }
  if (Str == "equip_on") {
    return CapabilityFlags::EquipOn;
  }
  if (Str == "unequip_from") {
    return CapabilityFlags::UnequipFrom;
  }
  if (Str == "dismantle") {
    return CapabilityFlags::Dismantle;
  }
  if (Str == "ranged") {
    return CapabilityFlags::Ranged;
  }
  if (Str == "ranged_use") {
    return CapabilityFlags::Ranged | CapabilityFlags::UseOn;
  }
  if (Str == "adjacent_use") {
    return CapabilityFlags::Adjacent | CapabilityFlags::UseOn;
  }
  return std::nullopt;
}
CapabilityFlags CapabilityFlags::fromString(const std::string &Str) {
  if (const auto It = parseString(Str); It.has_value()) {
    return It.value();
  }
  throw std::out_of_range("Unknown capability: " + Str);
}

const char *CapabilityFlags::str() const {
  if ((CapabilityFlags::UseOn | CapabilityFlags::Self) == Value) {
    return "Use";
  }
  if ((CapabilityFlags::UseOn | CapabilityFlags::Ranged) == Value) {
    return "Ranged use";
  }
  if ((CapabilityFlags::UseOn | CapabilityFlags::Adjacent) == Value) {
    return "Adjacent use";
  }
  switch (Value) {
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
  case CapabilityFlags::Dismantle:
    return "Dismantle";
  case CapabilityFlags::Ranged:
    return "Ranged";
  case CapabilityFlags::Adjacent:
    return "Adjacent";
  default:
    break;
  }
  return "<unimp. CapabilityFlags>";
}

std::ostream &operator<<(std::ostream &Out, const CapabilityFlags &Flags) {
  Out << Flags.str();
  return Out;
}

void EffectAttributes::updateCostsFrom(const EffectAttributes &Other) {
  APCost = std::max(APCost, Other.APCost);
  ManaCost = std::max(ManaCost, Other.ManaCost);
  HealthCost = std::max(HealthCost, Other.HealthCost);
}

} // namespace rogue