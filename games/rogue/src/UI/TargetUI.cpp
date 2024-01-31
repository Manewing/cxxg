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
#include <rogue/UI/CompHelpers.h>
#include <rogue/UI/Controller.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/ItemSelect.h>
#include <rogue/UI/TargetUI.h>

namespace rogue::ui {

bool TargetInfo::DebugTargets = false;

TargetInfo::TargetInfo(Controller &C, entt::entity TEt, Level &L)
    : BaseRectDecorator({2, 2}, {40, 10}, nullptr), Ctrl(C), TargetEt(TEt),
      Lvl(L), WW("", getSize().X - 2) {
  auto *NC = Lvl.Reg.try_get<NameComp>(TargetEt);
  std::string Hdr = NC ? NC->Name : "Unknown";
  std::string Desc = NC ? NC->Description : "";

  WW = WordWrap(Desc, getSize().X - 4);
  auto Offset = WW.getNumLines() + 3;
  Rect.setSize(getSize() + cxxg::types::Size{0, Offset});

  auto ItSel = std::make_shared<ItemSelect>(Pos);
  int Count = Offset;

  if (DebugTargets) {
    if (Lvl.Reg.try_get<EquipmentComp>(TargetEt)) {
      ItSel->addSelect<Select>(
          "Equip", cxxg::types::Position{Pos.X, Pos.Y + ++Count}, 12);
    }
    if (Lvl.Reg.try_get<InventoryComp>(TargetEt)) {
      ItSel->addSelect<Select>(
          "Inventory", cxxg::types::Position{Pos.X, Pos.Y + ++Count}, 12);
    }
  }

  if (Lvl.Reg.try_get<StatsComp>(TargetEt)) {
    ItSel->addSelect<Select>("Stats",
                             cxxg::types::Position{Pos.X, Pos.Y + ++Count}, 12);
  }
  if (BuffTypeList::anyOf(Lvl.Reg, TargetEt)) {
    ItSel->addSelect<Select>("Buffs",
                             cxxg::types::Position{Pos.X, Pos.Y + ++Count}, 25);
  }

  ItSel->registerOnSelectCallback([this](const auto &Sel) {
    // FIXME equip and inventory should only be allowed in debug mode
    if (Sel.getValue() == "Equip") {
      Ctrl.setEquipmentUI(TargetEt, Lvl);
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

  for (std::size_t Y = 0; Y < WW.getNumLines(); ++Y) {
    Scr[Pos.Y + Y + 1][Pos.X + 2] << WW.getLine(Y);
  }

  auto Offset = WW.getNumLines() + 2;
  auto DP = Pos + cxxg::types::Position{2, static_cast<int>(Offset)};
  addHealthInfo(Scr, DP, Lvl.Reg, TargetEt);
  DP.Y++;
  addManaInfo(Scr, DP, Lvl.Reg, TargetEt);
  DP.Y++;
  addAgilityInfo(Scr, DP, Lvl.Reg, TargetEt);
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