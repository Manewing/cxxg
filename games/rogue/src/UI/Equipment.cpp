#include <cxxg/Utils.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/LOS.h>
#include <rogue/Components/Transform.h>
#include <rogue/CraftingHandler.h>
#include <rogue/Item.h>
#include <rogue/Level.h>
#include <rogue/UI/Controller.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/Equipment.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/Item.h>
#include <rogue/UI/ItemSelect.h>
#include <rogue/UI/Tooltip.h>

namespace rogue::ui {

constexpr cxxg::types::Size TooltipSize = {40, 10};
constexpr cxxg::types::Position TooltipOffset = {4, 4};

bool EquipmentController::handleUseSkill(Controller &Ctrl, Level &Lvl,
                                         entt::entity Entity,
                                         const EquipmentSlot &ES) {
  auto &Reg = Lvl.Reg;
  const auto ItType = ES.BaseTypeFilter;
  if (ES.It && ES.It->getCapabilityFlags().isRanged(CapabilityFlags::Skill)) {
    auto &PC = Reg.get<PositionComp>(Entity);
    std::optional<unsigned> Range;
    if (auto *LOSComp = Reg.try_get<LineOfSightComp>(Entity)) {
      Range = LOSComp->LOSRange;
    }
    Ctrl.setTargetUI(PC.Pos, Range, Lvl,
                     [&R = Reg, E = Entity, Hub = Ctrl.getEventHub(),
                      ItType](auto TgEt, auto) -> void {
                       InventoryHandler IH(E, R, CraftingHandler());
                       IH.setEventHub(Hub);
                       IH.tryUseSkillOnTarget(ItType, TgEt);
                     });
    return false;
  }
  if (ES.It && ES.It->getCapabilityFlags().isAdjacent(CapabilityFlags::Skill)) {
    auto &PC = Reg.get<PositionComp>(Entity);
    Ctrl.setTargetUI(PC.Pos, /*Range=*/2, Lvl,
                     [&R = Reg, E = Entity, Hub = Ctrl.getEventHub(),
                      ItType](auto TgEt, auto) -> void {
                       InventoryHandler IH(E, R, CraftingHandler());
                       IH.setEventHub(Hub);
                       IH.tryUseSkillOnTarget(ItType, TgEt);
                     });
    return false;
  }

  InventoryHandler InvHandler(Entity, Reg, CraftingHandler());
  if (InvHandler.tryUseSkill(ItType)) {
    return true;
  }

  return false;
}

EquipmentController::EquipmentController(Controller &Ctrl, Equipment &Equip,
                                         entt::entity Entity, Level &Lvl,
                                         cxxg::types::Position Pos)
    : BaseRectDecorator(Pos, {40, 11}, nullptr), Ctrl(Ctrl), Equip(Equip),
      Entity(Entity), Lvl(Lvl), Reg(Lvl.Reg),
      InvHandler(Entity, Reg, CraftingHandler()) {
  InvHandler.setEventHub(Ctrl.getEventHub());
  ItSel = std::make_shared<ItemSelect>(Pos);
  Comp = std::make_shared<Frame>(ItSel, Pos, getSize(), "Equipment");

  int Count = 0;
  for (const auto *ES : Equip.all()) {
    addSelect(*ES, {Pos.X + 1, Pos.Y + Count++});
  }
}

bool EquipmentController::handleInput(int Char) {
  switch (Char) {
  case Controls::CloseWindow.Char:
  case Controls::EquipmentUI.Char:
    return false;
  case Controls::Info.Char: {
    auto SelIdx = ItSel->getSelectedIdx();
    auto *ES = Equip.all().at(SelIdx);
    if (ES->It) {
      Ctrl.addWindow(std::make_shared<ItemTooltip>(
          Pos + TooltipOffset, TooltipSize, *ES->It, /*Equipped=*/true));
    }
  } break;
  case Controls::Skill.Char: {
    auto SelIdx = ItSel->getSelectedIdx();
    auto *ES = Equip.all().at(SelIdx);
    return !handleUseSkill(Ctrl, Lvl, Entity, *ES);
  }
  case Controls::Unequip.Char: {
    const auto SelIdx = ItSel->getSelectedIdx();
    const auto *ES = Equip.all().at(SelIdx);
    InvHandler.tryUnequip(ES->BaseTypeFilter);
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
    if (ES->It->canRemoveFrom(Entity, Entity, Reg,
                              CapabilityFlags::UnequipFrom)) {
      Options.push_back(Controls::Unequip);
    }
    if (ES->It->getCapabilityFlags() & CapabilityFlags::Skill) {
      Options.push_back(Controls::Skill);
    }
  }

  return KeyOption::getInteractMsg(Options);
}

void EquipmentController::draw(cxxg::Screen &Scr) const {
  updateSelectValues();
  BaseRectDecorator::draw(Scr);
}

namespace {

std::string getSelectValue(const EquipmentSlot &ES, int Idx) {
  if (ES.It) {
    std::string Prefix = "[-] ";
    if (ES.It->hasEffect(CapabilityFlags::Skill)) {
      Prefix = "[" + std::to_string(Idx) + "] ";
    }
    return Prefix + ES.It->getQualifierName();
  }
  return "[-] ---";
}

cxxg::types::TermColor getSelectColor(const EquipmentSlot &ES) {
  if (ES.It) {
    return getColorForItem(*ES.It);
  }
  return cxxg::types::Color::NONE;
}

} // namespace

void EquipmentController::addSelect(const EquipmentSlot &ES,
                                    cxxg::types::Position AtPos) {
  constexpr const auto NoColor = cxxg::types::Color::NONE;
  int Idx = AtPos.Y - Pos.Y + 1;
  ItSel->addSelect<LabeledSelect>(ES.BaseTypeFilter.str(),
                                  getSelectValue(ES, Idx), AtPos,
                                  getSize().X - 2, NoColor, NoColor);
}

void EquipmentController::updateSelectValues() const {
  std::size_t Count = 0;
  for (const auto *ES : Equip.all()) {
    auto &Sel = ItSel->getSelect(Count++);
    Sel.setValue(getSelectValue(*ES, Count));
    Sel.setValueColor(getSelectColor(*ES));
  }
}

} // namespace rogue::ui