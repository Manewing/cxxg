#include <cxxg/Utils.h>
#include <iomanip>
#include <rogue/Components/Items.h>
#include <rogue/Components/Visual.h>
#include <rogue/Event.h>
#include <rogue/Inventory.h>
#include <rogue/UI/Controller.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/Inventory.h>
#include <rogue/UI/ListSelect.h>
#include <rogue/UI/Tooltip.h>

namespace rogue::ui {

constexpr cxxg::types::Size TooltipSize = {40, 10};
constexpr cxxg::types::Position TooltipOffset = {4, 4};

cxxg::types::TermColor
InventoryControllerBase::getColorForItemType(ItemType Type) {
  if ((Type & ItemType::Quest) != ItemType::None) {
    return cxxg::types::RgbColor{195, 196, 90};
  }
  if ((Type & ItemType::EquipmentMask) != ItemType::None) {
    return cxxg::types::RgbColor{227, 175, 91};
  }
  if ((Type & ItemType::Consumable) != ItemType::None) {
    return cxxg::types::RgbColor{112, 124, 219};
  }
  if ((Type & ItemType::Crafting) != ItemType::None) {
    return cxxg::types::RgbColor{182, 186, 214};
  }
  return cxxg::types::Color::NONE;
}

InventoryControllerBase::InventoryControllerBase(Controller &Ctrl,
                                                 Inventory &Inv,
                                                 entt::entity Entity,
                                                 entt::registry &Reg,
                                                 const std::string &Header)
    : BaseRectDecorator({2, 2}, {40, 18}, nullptr), Ctrl(Ctrl), Inv(Inv),
      Entity(Entity), Reg(Reg) {
  List = std::make_shared<ListSelect>(Pos, getSize());
  Comp = std::make_shared<Frame>(List, Pos, getSize(), Header);
  updateElements();
}

bool InventoryControllerBase::handleInput(int Char) {
  switch (Char) {
  case Controls::Info.Char: {
    const auto &It = Inv.getItem(List->getSelectedElement());
    Ctrl.addWindow(
        std::make_shared<ItemTooltip>(Pos + TooltipOffset, TooltipSize, It));
  } break;
  case 'i':
    return false;
  default:
    return List->handleInput(Char);
  }
  return true;
}

void InventoryControllerBase::draw(cxxg::Screen &Scr) const {
  updateElements();
  BaseRectDecorator::draw(Scr);
}

void InventoryControllerBase::updateElements() const {
  std::vector<ListSelect::Element> Elements;
  Elements.reserve(Inv.getItems().size());
  for (const auto &Item : Inv.getItems()) {
    std::stringstream SS;
    if (Item.getMaxStackSize() == 1) {
      SS << "     " << Item.getName();
    } else {
      SS << std::setw(3) << Item.StackSize << "x " << Item.getName();
    }
    Elements.push_back({SS.str(), getColorForItemType(Item.getType())});
  }
  auto PrevIdx = List->getSelectedElement();
  List->setElements(Elements);
  List->selectElement(PrevIdx);
}

InventoryController::InventoryController(Controller &Ctrl, Inventory &Inv,
                                         entt::entity Entity,
                                         entt::registry &Reg)
    : InventoryControllerBase(Ctrl, Inv, Entity, Reg, "Inventory") {}

bool InventoryController::handleInput(int Char) {
  if (Inv.empty()) {
    return InventoryControllerBase::handleInput(Char);
  }

  const auto ItemIdx = List->getSelectedElement();
  switch (Char) {
  case Controls::Equip.Char: {
    const auto &It = Inv.getItem(ItemIdx);
    auto Equip = Reg.try_get<EquipmentComp>(Entity);
    if (!Equip) {
      Ctrl.publish(PlayerInfoMessageEvent()
                   << "Can not equip " + It.getName() + " on " +
                          Reg.get<NameComp>(Entity).Name);
      break;
    }

    if (auto EquipItOrNone =
            Equip->Equip.tryUnequip(It.getType(), Entity, Reg)) {
      Inv.addItem(*EquipItOrNone);
    }

    if (!Equip->Equip.canEquip(It, Entity, Reg)) {
      Ctrl.publish(PlayerInfoMessageEvent()
                   << "Can not equip " + It.getName() + " on " +
                          Reg.get<NameComp>(Entity).Name);
      break;
    }

    Ctrl.publish(PlayerInfoMessageEvent()
                 << "Equip " + It.getName() + " on " +
                        Reg.get<NameComp>(Entity).Name);
    Equip->Equip.equip(Inv.takeItem(ItemIdx, /*Count=*/1), Entity, Reg);
    updateElements();
  } break;
  case Controls::Use.Char: {
    auto ItOrNone =
        Inv.applyItemTo(ItemIdx, CapabilityFlags::UseOn, Entity, Reg);
    if (ItOrNone) {
      // FIXME add information on result of use
      Ctrl.publish(PlayerInfoMessageEvent()
                   << "Used " + ItOrNone->getName() + " on " +
                          Reg.get<NameComp>(Entity).Name);
    } else {
      Ctrl.publish(PlayerInfoMessageEvent()
                   << "Can not use " + Inv.getItem(ItemIdx).getName() + " on " +
                          Reg.get<NameComp>(Entity).Name);
    }
    updateElements();
  } break;
  case Controls::Drop.Char:
    // Inv.dropItem(List.getSelectedElement());
    break;
  case Controls::Dismantle.Char: {
    auto ItOrNone =
        Inv.applyItemTo(ItemIdx, CapabilityFlags::Dismantle, Entity, Reg);
    if (ItOrNone) {
      // FIXME add information on result of dismantle
      Ctrl.publish(PlayerInfoMessageEvent()
                   << "Dismantled " + ItOrNone->getName() + " for " +
                          Reg.get<NameComp>(Entity).Name);
    } else {
      Ctrl.publish(PlayerInfoMessageEvent()
                   << "Cannot dismantle " + Inv.getItem(ItemIdx).getName() +
                          " for " + Reg.get<NameComp>(Entity).Name);
    }
    updateElements();
  } break;
  default:
    return InventoryControllerBase::handleInput(Char);
  }
  return true;
}

std::string InventoryController::getInteractMsg() const {
  if (Inv.empty()) {
    return "";
  }

  const auto &SelectedItem = Inv.getItem(List->getSelectedElement());
  std::vector<KeyOption> Options = {Controls::Info, Controls::Drop};
  if ((SelectedItem.getType() & ItemType::EquipmentMask) != ItemType::None) {
    Options.push_back(Controls::Equip);
  }
  if ((SelectedItem.getType() & ItemType::Crafting) != ItemType::None) {
    Options.push_back(Controls::Craft);
  }
  if ((SelectedItem.getType() & ItemType::Consumable) != ItemType::None) {
    Options.push_back(Controls::Use);
  }
  if ((SelectedItem.getCapabilityFlags() & CapabilityFlags::Dismantle) !=
      CapabilityFlags::None) {
    Options.push_back(Controls::Dismantle);
  }
  return KeyOption::getInteractMsg(Options);
}

LootController::LootController(Controller &Ctrl, Inventory &Inv,
                               entt::entity Entity, entt::registry &Reg)
    : InventoryControllerBase(Ctrl, Inv, Entity, Reg, "Loot") {}

bool LootController::handleInput(int Char) {
  switch (Char) {
  case 'e': {
    if (Inv.empty()) {
      break;
    }
    auto &EtInv = Reg.get<InventoryComp>(Entity).Inv;
    EtInv.addItem(Inv.takeItem(List->getSelectedElement()));
    updateElements();
  } break;
  default:
    return InventoryControllerBase::handleInput(Char);
  }
  return !Inv.empty();
}

std::string LootController::getInteractMsg() const {
  if (Inv.empty()) {
    return "";
  }
  std::vector<KeyOption> Options = {Controls::Info, Controls::Take};
  return KeyOption::getInteractMsg(Options);
}

} // namespace rogue::ui