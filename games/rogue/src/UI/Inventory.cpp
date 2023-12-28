#include <cxxg/Utils.h>
#include <iomanip>
#include <rogue/Components/Items.h>
#include <rogue/Components/LOS.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/CraftingHandler.h>
#include <rogue/Event.h>
#include <rogue/Inventory.h>
#include <rogue/Level.h>
#include <rogue/UI/Controller.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/Inventory.h>
#include <rogue/UI/Item.h>
#include <rogue/UI/ListSelect.h>
#include <rogue/UI/Tooltip.h>

namespace rogue::ui {

constexpr cxxg::types::Size TooltipSize = {40, 10};
constexpr cxxg::types::Position TooltipOffset = {4, 4};

InventoryControllerBase::InventoryControllerBase(Controller &Ctrl,
                                                 Inventory &Inv,
                                                 entt::entity Entity,
                                                 Level &Lvl,
                                                 const std::string &Header)
    : BaseRectDecorator({2, 2}, {40, 18}, nullptr), Ctrl(Ctrl), Inv(Inv),
      Entity(Entity), Lvl(Lvl), InvHandler(Entity, Lvl.Reg, CraftingHandler()) {
  InvHandler.setEventHub(Ctrl.getEventHub());
  List = std::make_shared<ListSelect>(Pos, getSize());
  Comp = std::make_shared<Frame>(List, Pos, getSize(), Header);
  updateElements();
}

bool InventoryControllerBase::handleInput(int Char) {
  switch (Char) {
  case Controls::Info.Char: {
    const auto &It = Inv.getItem(List->getSelectedElement());
    Ctrl.addWindow(std::make_shared<ItemTooltip>(
        Pos + TooltipOffset, TooltipSize, It, /*Equipped=*/false));

    // If the item is equipped, show the equipped item as well
    if (auto *EC = Lvl.Reg.try_get<EquipmentComp>(Entity)) {
      if (auto *EqIt = EC->Equip.getEquipped(It.getType())) {
        Ctrl.addWindow(std::make_shared<ItemTooltip>(
            Pos + TooltipOffset + cxxg::types::Position{TooltipSize.X + 1, 0},
            TooltipSize, *EqIt, /*Equipped=*/true));
      }
    }
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
    if (Item.getMaxStackSize() == 1 || Inv.getMaxStackSize() == 1) {
      SS << "     " << Item.getName();
    } else {
      SS << std::setw(3) << Item.StackSize << "x " << Item.getName();
    }
    Elements.push_back({SS.str(), getColorForItem(Item)});
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
    if (Inv.getItem(ItemIdx).getCapabilityFlags() & CapabilityFlags::Ranged) {
      auto &PC = Lvl.Reg.get<PositionComp>(Entity);
      std::optional<unsigned> Range;
      if (auto *LOSComp = Lvl.Reg.try_get<LineOfSightComp>(Entity)) {
        Range = LOSComp->LOSRange;
      }
      Ctrl.setTargetUI(PC.Pos, Range, Lvl,
                       [&R = Lvl.Reg, E = Entity, Hub = Ctrl.getEventHub(),
                        ItemIdx](auto TgEt, auto) -> void {
                         InventoryHandler IH(E, R, CraftingHandler());
                         IH.setEventHub(Hub);
                         IH.tryUseItemOnTarget(ItemIdx, TgEt);
                       });
      return false;
    }
    if (Inv.getItem(ItemIdx).getCapabilityFlags() & CapabilityFlags::Adjacent) {
      auto &PC = Lvl.Reg.get<PositionComp>(Entity);
      Ctrl.setTargetUI(PC.Pos, /*Range=*/2, Lvl,
                       [&R = Lvl.Reg, E = Entity, Hub = Ctrl.getEventHub(),
                        ItemIdx](auto TgEt, auto) -> void {
                         InventoryHandler IH(E, R, CraftingHandler());
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
  case Controls::StoreOne.Char:
    if (auto *LUI = Ctrl.getWindowOfType<LootController>()) {
      LUI->getInventory().addItem(Inv.takeItem(ItemIdx, 1));
    }
    break;
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
  // FIXME this should all be based on capability flags, not item type
  if (SelectedItem.getType() & ItemType::EquipmentMask) {
    Options.push_back(Controls::Equip);
  }
  if (SelectedItem.getType() & ItemType::Crafting) {
    Options.push_back(Controls::Craft);
  }
  if (SelectedItem.getType() & ItemType::Consumable) {
    Options.push_back(Controls::Use);
  }
  if (SelectedItem.getCapabilityFlags() & CapabilityFlags::Dismantle) {
    Options.push_back(Controls::Dismantle);
  }
  if (Ctrl.hasLootUI()) {
    Options.push_back(Controls::Store);
    Options.push_back(Controls::StoreOne);
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
  case Controls::TakeOne.Char: {
    if (Inv.empty()) {
      break;
    }
    auto &EtInv = Lvl.Reg.get<InventoryComp>(Entity).Inv;
    EtInv.addItem(Inv.takeItem(List->getSelectedElement(), 1));
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
  std::vector<KeyOption> Options = {Controls::Info, Controls::Take,
                                    Controls::TakeOne};
  return KeyOption::getInteractMsg(Options);
}

} // namespace rogue::ui