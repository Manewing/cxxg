#include <cxxg/Utils.h>
#include <rogue/Components/Items.h>
#include <rogue/Item.h>
#include <rogue/UI/Equipment.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/ItemSelect.h>

namespace rogue::ui {

namespace {

const char *getItemTypeLabel(ItemType It) {
  switch (It & ItemType::EquipmentMask) {
  case ItemType::Amulet:
    return "Amulet";
  case ItemType::ChestPlate:
    return "Chest Plate";
  case ItemType::Boots:
    return "Boots";
  case ItemType::Ring:
    return "Ring";
  case ItemType::Helmet:
    return "Helmet";
  case ItemType::Pants:
    return "Pants";
  case ItemType::Weapon:
    return "Weapon";
  case ItemType::OffHand:
    return "Off Hand";
  default:
    break;
  }
  return "<unimp. ItemType>";
}

} // namespace

EquipmentController::EquipmentController(Equipment &Equip, entt::entity Entity,
                                         entt::registry &Reg,
                                         cxxg::types::Position Pos)
    : BaseRect(Pos, {50, 10}), Equip(Equip), Entity(Entity), Reg(Reg) {
  ItSel = std::make_shared<ItemSelect>();
  Dec = std::make_shared<Frame>(ItSel, Pos, Size, "Equipment");

  int Count = 0;
  for (const auto *ES : Equip.all()) {
    auto PosX = 1 + Pos.X + (Count / 4) * int(Size.X / 2);
    auto PosY = 1 + Pos.Y + Count % 4;
    Count++;
    addSelect(*ES, {PosX, PosY});
  }
}

bool EquipmentController::handleInput(int Char) {
  switch (Char) {
  case cxxg::utils::KEY_ESC:
    return false;
  case 'o':
    return false;
  case 'u': {
    auto InvComp = Reg.try_get<InventoryComp>(Entity);
    if (!InvComp) {
      // FIXME message
      break;
    }
    auto SelIdx = ItSel->getSelectedIdx();
    auto *ES = Equip.all().at(SelIdx);
    if (!ES->It ||
        !ES->It->canRemoveFrom(Entity, Reg, CapabilityFlags::UnequipFrom)) {
      // FIXME message
      break;
    }
    ES->It->removeFrom(Entity, Reg, CapabilityFlags::UnequipFrom);
    InvComp->Inv.addItem(Equip.unequip(ES->BaseTypeFilter));
    updateSelectValues();
  } break;

  default:
    return ItSel->handleInput(Char);
  }
  return true;
}

std::string_view EquipmentController::getInteractMsg() const {
  return "[^v] Nav.";
}

void EquipmentController::draw(cxxg::Screen &Scr) const {
  BaseRect::draw(Scr);
  Dec->draw(Scr);
}

namespace {

std::string getSelectValue(const EquipmentSlot &ES) {
  if (ES.It) {
    return ES.It->getName();
  }
  return "---";
}

} // namespace

void EquipmentController::addSelect(const EquipmentSlot &ES,
                                    cxxg::types::Position Pos) {
  ItSel->addSelect(std::make_shared<LabeledSelect>(
      getItemTypeLabel(ES.BaseTypeFilter), getSelectValue(ES), Pos, 25));
}

void EquipmentController::updateSelectValues() {
  std::size_t Count = 0;
  for (const auto *ES : Equip.all()) {
    ItSel->getSelect(Count++).setValue(getSelectValue(*ES));
  }
}

} // namespace rogue::ui