#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <rogue/UI/Decorator.h>
#include <rogue/UI/Controls.h>

namespace rogue::ui {

Decorator::Decorator(cxxg::types::Position Pos, std::shared_ptr<Widget> Comp)
    : Widget(Pos), Comp(std::move(Comp)) {}

void Decorator::setPos(cxxg::types::Position P) {
  Pos = P;
  Comp->setPos(P);
}

void Decorator::setComp(std::shared_ptr<Widget> Comp) {
  this->Comp = std::move(Comp);
}

bool Decorator::handleInput(int Char) { return Comp->handleInput(Char); }

std::string Decorator::getInteractMsg() const { return Comp->getInteractMsg(); }

void Decorator::draw(cxxg::Screen &Scr) const { Comp->draw(Scr); }

BaseRectDecorator::BaseRectDecorator(cxxg::types::Position Pos,
                                     cxxg::types::Size Size,
                                     std::shared_ptr<Widget> Comp)
    : Decorator(Pos, std::move(Comp)), Rect(Pos, Size) {}

void BaseRectDecorator::setPos(cxxg::types::Position Pos) {
  Decorator::setPos(Pos);
  Rect.setPos(Pos);
}

void BaseRectDecorator::draw(cxxg::Screen &Scr) const {
  Rect.draw(Scr);
  Decorator::draw(Scr);
}

void ExitHandlerProvider::registerExitHandler(ExitHandlerCallback EMC) {
  this->EMC = std::move(EMC);
}

void ExitHandlerProvider::handleExit() { EMC(); }

MoveDecorator::MoveDecorator(const std::shared_ptr<Widget> &Comp)
    : Decorator(Comp->getPos(), Comp) {}

bool MoveDecorator::handleInput(int Char) {
  switch (Char) {
  case Controls::CloseWindow.Char:
    handleExit();
    break;
  case Controls::MoveLeft.Char:
    setPos(Pos + cxxg::types::Position{-1, 0});
    break;
  case Controls::MoveRight.Char:
    setPos(Pos + cxxg::types::Position{1, 0});
    break;
  case Controls::MoveUp.Char:
    setPos(Pos + cxxg::types::Position{0, -1});
    break;
  case Controls::MoveDown.Char:
    setPos(Pos + cxxg::types::Position{0, 1});
    break;
  default:
    break;
  }
  return true;
}

void MoveDecorator::draw(cxxg::Screen &Scr) const {
  Decorator::draw(Scr);
  Scr[Pos.Y][Pos.X] << IconColor << "X";
}

} // namespace rogue::ui