#include <cxxg/Utils.h>
#include <iomanip>
#include <rogue/Components/Entity.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/LOS.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Event.h>
#include <rogue/Inventory.h>
#include <rogue/Level.h>
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
                                                 Level &Lvl,
                                                 const std::string &Header)
    : BaseRectDecorator({2, 2}, {40, 18}, nullptr), Ctrl(Ctrl), Inv(Inv),
      Entity(Entity), Lvl(Lvl), InvHandler(Entity, Lvl.Reg) {
  InvHandler.setEventHub(Ctrl.getEventHub());
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
  case Controls::CloseWindow.Char:
  case Controls::InventoryUI.Char:
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

Inventory &InventoryControllerBase::getInventory() { return Inv; }

const Inventory &InventoryControllerBase::getInventory() const { return Inv; }

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
                                         entt::entity Entity, Level &Lvl)
    : InventoryControllerBase(Ctrl, Inv, Entity, Lvl, "Inventory") {}

bool InventoryController::handleInput(int Char) {
  if (Inv.empty()) {
    return InventoryControllerBase::handleInput(Char);
  }

  const auto ItemIdx = List->getSelectedElement();
  switch (Char) {
  case Controls::Equip.Char: {
    InvHandler.tryEquipItem(ItemIdx);
  } break;
  case Controls::Use.Char: {
    if ((Inv.getItem(ItemIdx).getCapabilityFlags() & CapabilityFlags::Ranged) !=
        CapabilityFlags::None) {
      auto &PC = Lvl.Reg.get<PositionComp>(Entity);
      std::optional<unsigned> Range;
      if (auto *LOSComp = Lvl.Reg.try_get<LineOfSightComp>(Entity)) {
        Range = LOSComp->LOSRange;
      }
      Ctrl.setTargetUI(PC.Pos, Range, Lvl,
                       [&R = Lvl.Reg, E = Entity, Hub = Ctrl.getEventHub(),
                        ItemIdx](auto TgEt, auto) -> void {
                         InventoryHandler IH(E, R);
                         IH.setEventHub(Hub);
                         IH.tryUseItemOnTarget(ItemIdx, TgEt);
                       });
      return false;
    }
    if ((Inv.getItem(ItemIdx).getCapabilityFlags() &
         CapabilityFlags::Adjacent) != CapabilityFlags::None) {
      auto &PC = Lvl.Reg.get<PositionComp>(Entity);
      Ctrl.setTargetUI(PC.Pos, /*Range=*/2, Lvl,
                       [&R = Lvl.Reg, E = Entity, Hub = Ctrl.getEventHub(),
                        ItemIdx](auto TgEt, auto) -> void {
                         InventoryHandler IH(E, R);
                         IH.setEventHub(Hub);
                         IH.tryUseItemOnTarget(ItemIdx, TgEt);
                       });
      return false;
    }
    InvHandler.tryUseItem(ItemIdx);
  } break;
  case Controls::Drop.Char: {
    InvHandler.tryDropItem(ItemIdx);
  } break;
  case Controls::Dismantle.Char: {
    InvHandler.tryDismantleItem(ItemIdx);
  } break;
  case Controls::Store.Char:
    if (auto *LUI = Ctrl.getWindowOfType<LootController>()) {
      LUI->getInventory().addItem(Inv.takeItem(ItemIdx));
    }
    break;
  default:
    return InventoryControllerBase::handleInput(Char);
  }
  updateElements();
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
  if (Ctrl.hasLootUI()) {
    Options.push_back(Controls::Store);
  }
  return KeyOption::getInteractMsg(Options);
}

LootController::LootController(Controller &Ctrl, Inventory &Inv,
                               entt::entity Entity, Level &Lvl,
                               const std::string &Header)
    : InventoryControllerBase(Ctrl, Inv, Entity, Lvl, Header) {}

bool LootController::handleInput(int Char) {
  switch (Char) {
  case Controls::Take.Char: {
    if (Inv.empty()) {
      break;
    }
    auto &EtInv = Lvl.Reg.get<InventoryComp>(Entity).Inv;
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