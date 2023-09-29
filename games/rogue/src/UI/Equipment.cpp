#include <cxxg/Utils.h>
#include <rogue/Components/Items.h>
#include <rogue/Item.h>
#include <rogue/UI/Controller.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/Equipment.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/Inventory.h>
#include <rogue/UI/ItemSelect.h>
#include <rogue/UI/Tooltip.h>

namespace rogue::ui {

constexpr cxxg::types::Size TooltipSize = {40, 10};
constexpr cxxg::types::Position TooltipOffset = {4, 4};

EquipmentController::EquipmentController(Controller &Ctrl, Equipment &Equip,
                                         entt::entity Entity,
                                         entt::registry &Reg,
                                         cxxg::types::Position Pos)
    : BaseRectDecorator(Pos, {40, 11}, nullptr), Ctrl(Ctrl), Equip(Equip),
      Entity(Entity), Reg(Reg) {
  ItSel = std::make_shared<ItemSelect>(Pos);
  Comp = std::make_shared<Frame>(ItSel, Pos, getSize(), "Equipment");

  int Count = 0;
  for (const auto *ES : Equip.all()) {
    addSelect(*ES, {Pos.X + 1, Pos.Y + Count++});
  }
}

bool EquipmentController::handleInput(int Char) {
  switch (Char) {
  case cxxg::utils::KEY_ESC:
    return false;
  case 'o':
    return false;
  case Controls::Info.Char: {
    auto SelIdx = ItSel->getSelectedIdx();
    auto *ES = Equip.all().at(SelIdx);
    if (ES->It) {
      Ctrl.addWindow(std::make_shared<ItemTooltip>(Pos + TooltipOffset,
                                                   TooltipSize, *ES->It));
    }
  } break;
  case Controls::Unequip.Char: {
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
    InvComp->Inv.addItem(ES->unequip());
    updateSelectValues();
  } break;

  default:
    return ItSel->handleInput(Char);
  }
  return true;
}

std::string EquipmentController::getInteractMsg() const {
  std::vector<KeyOption> Options = {Controls::Navigate};

  auto SelIdx = ItSel->getSelectedIdx();
  auto *ES = Equip.all().at(SelIdx);

  if (ES->It) {
    Options.push_back(Controls::Info);
    if (ES->It->canRemoveFrom(Entity, Reg, CapabilityFlags::UnequipFrom)) {
      Options.push_back(Controls::Unequip);
    }
  }

  return KeyOption::getInteractMsg(Options);
}

void EquipmentController::draw(cxxg::Screen &Scr) const {
  updateSelectValues();
  BaseRectDecorator::draw(Scr);
}

namespace {

std::string getSelectValue(const EquipmentSlot &ES) {
  if (ES.It) {
    return ES.It->getName();
  }
  return "---";
}

cxxg::types::TermColor getSelectColor(const EquipmentSlot &ES) {
  if (ES.It) {
    return InventoryControllerBase::getColorForItemType(ES.It->getType());
  }
  return cxxg::types::Color::NONE;
}

} // namespace

void EquipmentController::addSelect(const EquipmentSlot &ES,
                                    cxxg::types::Position Pos) {
  constexpr const auto NoColor = cxxg::types::Color::NONE;
  ItSel->addSelect<LabeledSelect>(getItemTypeLabel(ES.BaseTypeFilter),
                                  getSelectValue(ES), Pos, 25, NoColor,
                                  getSelectColor(ES));
}

void EquipmentController::updateSelectValues() const {
  std::size_t Count = 0;
  for (const auto *ES : Equip.all()) {
    auto &Sel = ItSel->getSelect(Count++);
    Sel.setValue(getSelectValue(*ES));
    Sel.setValueColor(getSelectColor(*ES));
  }
}

} // namespace rogue::ui