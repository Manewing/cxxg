#include <map>
#include <rogue/ItemType.h>
#include <sstream>
#include <stdexcept>

namespace rogue {

ItemType getItemType(const std::string &Type) {
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
      {"generic", ItemType::Generic},
      {"consumable", ItemType::Consumable},
      {"quest", ItemType::Quest},
      {"crafting", ItemType::Crafting},
  };
  if (const auto It = ItemTypes.find(Type); It != ItemTypes.end()) {
    return It->second;
  }
  throw std::out_of_range("Unknown item type: " + std::string(Type));
  return ItemType::None;
}

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
  if ((It & ItemType::Shield) != ItemType::None) {
    Label << Pred << "Shield";
    Pred = ", ";
  }
  if ((It & ItemType::Ranged) != ItemType::None) {
    Label << Pred << "Ranged";
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

CapabilityFlags getCapabilityFlag(const std::string &CapabilityFlagStr) {
  if (CapabilityFlagStr == "use_on") {
    return CapabilityFlags::UseOn | CapabilityFlags::Self;
  }
  if (CapabilityFlagStr == "equipment") {
    return CapabilityFlags::Equipment;
  }
  if (CapabilityFlagStr == "equip_on") {
    return CapabilityFlags::EquipOn;
  }
  if (CapabilityFlagStr == "unequip_from") {
    return CapabilityFlags::UnequipFrom;
  }
  if (CapabilityFlagStr == "dismantle") {
    return CapabilityFlags::Dismantle;
  }
  if (CapabilityFlagStr == "ranged") {
    return CapabilityFlags::Ranged;
  }
  if (CapabilityFlagStr == "ranged_use") {
    return CapabilityFlags::Ranged | CapabilityFlags::UseOn;
  }
  if (CapabilityFlagStr == "adjacent_use") {
    return CapabilityFlags::Adjacent | CapabilityFlags::UseOn;
  }
  throw std::out_of_range("Unknown capability: " + CapabilityFlagStr);
  return CapabilityFlags::None;
}

const char *getCapabilityFlagLabel(CapabilityFlags Flags) {
  if ((CapabilityFlags::UseOn | CapabilityFlags::Self) == Flags) {
    return "Use";
  }
  if ((CapabilityFlags::UseOn | CapabilityFlags::Ranged)== Flags) {
    return "Ranged use";
  }
  if ((CapabilityFlags::UseOn | CapabilityFlags::Adjacent)== Flags) {
    return "Adjacent use";
  }
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

} // namespace rogue