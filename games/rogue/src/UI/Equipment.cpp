#include <cxxg/Utils.h>
#include <rogue/Components/Items.h>
#include <rogue/Item.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/Controller.h>
#include <rogue/UI/Equipment.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/ItemSelect.h>
#include <rogue/UI/Tooltip.h>

namespace rogue::ui {

constexpr cxxg::types::Size TooltipSize = {40, 10};
constexpr cxxg::types::Position TooltipOffset = {4, 4};

EquipmentController::EquipmentController(Controller &Ctrl, Equipment &Equip,
                                         entt::entity Entity,
                                         entt::registry &Reg,
                                         cxxg::types::Position Pos)
    : BaseRect(Pos, {40, 11}), Ctrl(Ctrl), Equip(Equip), Entity(Entity),
      Reg(Reg) {
  ItSel = std::make_shared<ItemSelect>(Pos);
  Dec = std::make_shared<Frame>(ItSel, Pos, Size, "Equipment");

  int Count = 0;
  for (const auto *ES : Equip.all()) {
    addSelect(*ES, {Pos.X + 1, Pos.Y + Count++});
  }
}

void EquipmentController::setPos(cxxg::types::Position Pos) {
  BaseRect::setPos(Pos);
  Dec->setPos(Pos);
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
    InvComp->Inv.addItem(Equip.unequip(ES->BaseTypeFilter));
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
  ItSel->addSelect<LabeledSelect>(getItemTypeLabel(ES.BaseTypeFilter),
                                  getSelectValue(ES), Pos, 25);
}

void EquipmentController::updateSelectValues() const {
  std::size_t Count = 0;
  for (const auto *ES : Equip.all()) {
    ItSel->getSelect(Count++).setValue(getSelectValue(*ES));
  }
}

} // namespace rogue::ui