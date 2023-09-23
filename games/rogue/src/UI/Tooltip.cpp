#include <memory>
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

const char *getItemTypeLabel(ItemType It) {
  switch (It & ItemType::EquipmentMask) {
  case ItemType::Ring:
    return "Ring";
  case ItemType::Amulet:
    return "Amulet";
  case ItemType::Helmet:
    return "Helmet";
  case ItemType::ChestPlate:
    return "Chest Plate";
  case ItemType::Pants:
    return "Pants";
  case ItemType::Boots:
    return "Boots";
  case ItemType::Weapon:
    return "Weapon";
  case ItemType::OffHand:
    return "Off Hand";
  case ItemType::Generic:
    return "Generic";
  case ItemType::Consumable:
    return "Consumable";
  case ItemType::Quest:
    return "Quest";
  case ItemType::Crafting:
    return "Crafting";
  default:
    break;
  }
  return "<unimp. ItemType>";
}

std::string getItemText(const Item &It) {
  std::stringstream SS;
  SS << "Name: " << It.getName() << "\n"
     << "Type: " << getItemTypeLabel(It.getType()) << "\n"
     << "---\n"
     << It.getDescription();
  return SS.str();
}

} // namespace

ItemTooltip::ItemTooltip(cxxg::types::Position Pos, cxxg::types::Size Size,
                         const Item &It)
    : Tooltip(Pos, Size, getItemText(It), It.getName()) {}

} // namespace rogue::ui