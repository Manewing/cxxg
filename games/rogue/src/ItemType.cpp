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

bool ItemType::is(ItemType Other) const {
  return (Value & Other.Value) == Other.Value;
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
  if (Str == "ranged_use") {
    return CapabilityFlags::Ranged | CapabilityFlags::UseOn;
  }
  if (Str == "adjacent_use") {
    return CapabilityFlags::Adjacent | CapabilityFlags::UseOn;
  }
  if (Str == "skill") {
    return CapabilityFlags::Skill | CapabilityFlags::Self;
  }
  if (Str == "skill_ranged") {
    return CapabilityFlags::Skill | CapabilityFlags::Ranged;
  }
  if (Str == "skill_adjacent") {
    return CapabilityFlags::Skill | CapabilityFlags::Adjacent;
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
  if ((CapabilityFlags::Skill | CapabilityFlags::Self) == Value) {
    return "Skill";
  }
  if ((CapabilityFlags::Skill | CapabilityFlags::Ranged) == Value) {
    return "Ranged skill";
  }
  if ((CapabilityFlags::Skill | CapabilityFlags::Adjacent) == Value) {
    return "Adjacent skill";
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
  case CapabilityFlags::Self:
    return "Self";
  case CapabilityFlags::Skill:
    return "Skill";
  default:
    break;
  }
  return "<unimp. CapabilityFlags>";
}

std::string CapabilityFlags::flagString() const {
  std::stringstream Flags;
  const char *Pred = "";
  if (Value & CapabilityFlags::UseOn) {
    Flags << Pred << "Use";
    Pred = " | ";
  }
  if (Value & CapabilityFlags::EquipOn) {
    Flags << Pred << "Equip";
    Pred = " | ";
  }
  if (Value & CapabilityFlags::UnequipFrom) {
    Flags << Pred << "Unequip";
    Pred = " | ";
  }
  if (Value & CapabilityFlags::Dismantle) {
    Flags << Pred << "Dismantle";
    Pred = " | ";
  }
  if (Value & CapabilityFlags::Ranged) {
    Flags << Pred << "Ranged";
    Pred = " | ";
  }
  if (Value & CapabilityFlags::Adjacent) {
    Flags << Pred << "Adjacent";
    Pred = " | ";
  }
  if (Value & CapabilityFlags::Self) {
    Flags << Pred << "Self";
    Pred = " | ";
  }
  if (Value & CapabilityFlags::Skill) {
    Flags << Pred << "Skill";
    Pred = " | ";
  }
  auto FlagsStr = Flags.str();
  if (FlagsStr.empty()) {
    return "<unimp. CapabilityFlags>";
  }
  return FlagsStr;
}

CapabilityFlags::operator bool() const {
  // Filter out capability flags that are not valid on their own
  return (Value & ~(Self | Ranged | Adjacent)) != None;
}

bool CapabilityFlags::is(CapabilityFlags Other) const {
  return (Value & Other) == Other;
}

bool CapabilityFlags::isAdjacent(CapabilityFlags Other) const {
  return (Value & (Adjacent | Other)) == (Adjacent | Other);
}

bool CapabilityFlags::isRanged(CapabilityFlags Other) const {
  return (Value & (Ranged | Other)) == (Ranged | Other);
}

std::ostream &operator<<(std::ostream &Out, const CapabilityFlags &Flags) {
  Out << Flags.str();
  return Out;
}

} // namespace rogue