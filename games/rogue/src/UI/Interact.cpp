#include <rogue/Components/Player.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Level.h>
#include <rogue/UI/Controller.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/Interact.h>
#include <rogue/UI/ListSelect.h>

namespace rogue::ui {

Interact::Interact(entt::entity SrcEt, ymir::Point2d<int> StartPos, Level &Lvl)
    : BaseRectDecorator({2, 2}, {20, 8}, nullptr), Lvl(Lvl), StartPos(StartPos),
      SrcEt(SrcEt) {
  InteractablesEts = Lvl.getInteractables(StartPos);
  List = std::make_shared<ListSelect>(Pos, getSize());
  Comp = std::make_shared<Frame>(List, Pos, getSize(), "Interact");
  updateElements();
  updateCursor();
}

bool Interact::handleInput(int Char) {
  bool KeepWindow = true;
  switch (Char) {
  case Controls::Interact.Char:
    destroyCursor();
    handleInteraction();
    return false;
  default:
    KeepWindow = List->handleInput(Char);
    updateCursor();
    break;
  }
  if (!KeepWindow) {
    destroyCursor();
  }
  return KeepWindow;
}

std::string Interact::getInteractMsg() const {
  return Controls::Interact.getInteractMsg();
}

void Interact::draw(cxxg::Screen &Scr) const { BaseRectDecorator::draw(Scr); }

void Interact::destroyCursor() {
  if (CursorEt != entt::null) {
    Lvl.Reg.destroy(CursorEt);
    CursorEt = entt::null;
  }
}

void Interact::updateElements() const {
  std::vector<ListSelect::Element> Elements;
  Elements.reserve(InteractablesEts.size());
  for (const auto &Entity : InteractablesEts) {
    auto &Interactable = Lvl.Reg.get<InteractableComp>(Entity);
    Elements.push_back({Interactable.Action.Msg, cxxg::types::Color::NONE});
  }

  auto PrevIdx = List->getSelectedElement();
  List->setElements(Elements);
  List->selectElement(PrevIdx);
}

void Interact::updateCursor() {
  if (CursorEt == entt::null) {
    CursorEt = createCursor(Lvl.Reg, StartPos);
  }
  auto SelIdx = List->getSelectedElement();
  auto &CursorPos = Lvl.Reg.get<PositionComp>(CursorEt).Pos;
  CursorPos = Lvl.Reg.get<PositionComp>(InteractablesEts.at(SelIdx)).Pos;
}

void Interact::handleInteraction() {
  auto &PC = Lvl.Reg.get<PlayerComp>(SrcEt);
  auto SelIdx = List->getSelectedElement();
  const auto InteractableEntity = InteractablesEts.at(SelIdx);
  auto &Interactable = Lvl.Reg.get<InteractableComp>(InteractableEntity);
  PC.CurrentInteraction = Interactable.Action;

  // This may switch level so needs to be last thing that is done
  Interactable.Action.Execute(Lvl, SrcEt, Lvl.Reg);
}

} // namespace rogue::ui