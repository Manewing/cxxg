#include <cxxg/Screen.h>
#include <cxxg/Types.h>
#include <cxxg/Utils.h>
#include <rogue/Components/AI.h>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Level.h>
#include <rogue/UI/Controller.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/ItemSelect.h>
#include <rogue/UI/TargetUI.h>

namespace rogue::ui {

TargetInfo::TargetInfo(Controller &C, entt::entity TEt, Level &L)
    : BaseRectDecorator({2, 2}, {25, 10}, nullptr), Ctrl(C), TargetEt(TEt),
      Lvl(L) {
  auto *NC = Lvl.Reg.try_get<NameComp>(TargetEt);
  std::string Hdr = NC ? NC->Name : "Unknown";

  auto ItSel = std::make_shared<ItemSelect>(Pos);
  int Count = 2;
  if (Lvl.Reg.try_get<EquipmentComp>(TargetEt)) {
    ItSel->addSelect<Select>(
        "Equip", cxxg::types::Position{Pos.X + 1, Pos.Y + ++Count}, 12);
  }
  if (Lvl.Reg.try_get<InventoryComp>(TargetEt)) {
    ItSel->addSelect<Select>(
        "Inventory", cxxg::types::Position{Pos.X + 1, Pos.Y + ++Count}, 12);
  }
  if (Lvl.Reg.try_get<StatsComp>(TargetEt)) {
    ItSel->addSelect<Select>(
        "Stats", cxxg::types::Position{Pos.X + 1, Pos.Y + ++Count}, 12);
  }
  if (BuffTypeList::anyOf(Lvl.Reg, TargetEt)) {
    ItSel->addSelect<Select>(
        "Buffs", cxxg::types::Position{Pos.X + 1, Pos.Y + ++Count}, 25);
  }

  ItSel->registerOnSelectCallback([this](const auto &Sel) {
    if (Sel.getValue() == "Equip") {
      Ctrl.setEquipmentUI(TargetEt, Lvl.Reg);
    }
    if (Sel.getValue() == "Buffs") {
      Ctrl.setBuffUI(TargetEt, Lvl.Reg);
    }
    if (Sel.getValue() == "Inventory") {
      Ctrl.setInventoryUI(TargetEt, Lvl);
    }
    if (Sel.getValue() == "Stats") {
      Ctrl.setStatsUI(TargetEt, Lvl.Reg);
    }
  });

  Comp = std::make_shared<Frame>(ItSel, Pos, getSize(), Hdr);
}

void TargetInfo::draw(cxxg::Screen &Scr) const {
  BaseRectDecorator::draw(Scr);

  cxxg::types::Position DrawPos = Pos + cxxg::types::Position{2, 1};

  // FIXME move this to general stats?
  if (auto *HC = Lvl.Reg.try_get<HealthComp>(TargetEt)) {
    Scr[DrawPos.Y][DrawPos.X + 1] << "Health: " << HC->Value << "/"
                                  << HC->MaxValue;
    DrawPos += cxxg::types::Position{0, 1};
  }
  if (auto *AG = Lvl.Reg.try_get<AgilityComp>(TargetEt)) {
    Scr[DrawPos.Y][DrawPos.X + 1] << "AP: " << AG->AP << " AG: " << AG->Agility;
    DrawPos += cxxg::types::Position{0, 1};
  }
  if (auto *AI = Lvl.Reg.try_get<WanderAIComp>(TargetEt)) {
    Scr[DrawPos.Y][DrawPos.X + 1] << "AI: " << getWanderAIStateStr(AI->State);
    DrawPos += cxxg::types::Position{0, 1};
  }
}

TargetUI::TargetUI(Controller &Ctrl, ymir::Point2d<int> StartPos,
                   std::optional<unsigned> Range, Level &Lvl,
                   const SelectTargetCb &Cb)
    : Widget({0, 0}), Ctrl(Ctrl), StartPos(StartPos), Range(Range), Lvl(Lvl),
      SelectCb(Cb) {
  CursorEt = createCursor(Lvl.Reg, StartPos);
  TargetEt = Lvl.getEntityAt(StartPos);
}

bool TargetUI::handleInput(int Char) {
  if (CursorEt == entt::null) {
    return true;
  }

  // Move cursor upon input
  auto &TargetPos = Lvl.Reg.get<PositionComp>(CursorEt).Pos;
  switch (Char) {
  case Controls::CloseWindow.Char:
    destroyCursor();
    return false;
  case Controls::MoveDown.Char:
    if (!Range ||
        static_cast<unsigned>(
            (TargetPos - StartPos + ymir::Point2d<int>{0, 1}).length()) <
            *Range) {
      TargetPos.Y++;
    }
    break;
  case Controls::MoveUp.Char:
    if (!Range ||
        static_cast<unsigned>(
            (TargetPos - StartPos + ymir::Point2d<int>{0, -1}).length()) <
            *Range) {
      TargetPos.Y--;
    }
    break;
  case Controls::MoveLeft.Char:
    if (!Range ||
        static_cast<unsigned>(
            (TargetPos - StartPos + ymir::Point2d<int>{-1, 0}).length()) <
            *Range) {
      TargetPos.X--;
    }
    break;
  case Controls::MoveRight.Char:
    if (!Range ||
        static_cast<unsigned>(
            (TargetPos - StartPos + ymir::Point2d<int>{1, 0}).length()) <
            *Range) {
      TargetPos.X++;
    }
    break;
  case Controls::Info.Char:
    if (TargetEt != entt::null) {
      showInfoForTarget(TargetEt);
    }
    break;
  case Controls::SelectTarget.Char: {
    SelectCb(TargetEt, TargetPos);
    destroyCursor();
    return false;
  }
  default:
    break;
  }

  TargetEt = Lvl.getEntityAt(TargetPos);

  return true;
}

std::string TargetUI::getInteractMsg() const {
  std::vector<KeyOption> Options = {Controls::MoveUp, Controls::MoveDown,
                                    Controls::MoveLeft, Controls::MoveRight,
                                    Controls::SelectTarget};
  if (TargetEt != entt::null) {
    Options.push_back(Controls::Info);
  }
  return KeyOption::getInteractMsg(Options);
}

void TargetUI::draw(cxxg::Screen &Scr) const { (void)Scr; }

ymir::Point2d<int> TargetUI::getCursorPos() const {
  if (CursorEt == entt::null) {
    return StartPos;
  }
  return Lvl.Reg.get<PositionComp>(CursorEt).Pos;
}

void TargetUI::destroyCursor() {
  if (CursorEt != entt::null) {
    Lvl.Reg.destroy(CursorEt);
    CursorEt = entt::null;
  }
}

void TargetUI::showInfoForTarget(entt::entity TargetEt) {
  Ctrl.addWindow(std::make_shared<TargetInfo>(Ctrl, TargetEt, Lvl));
}

} // namespace rogue::ui