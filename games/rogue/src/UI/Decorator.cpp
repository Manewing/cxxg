#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <rogue/UI/Decorator.h>

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

std::string_view Decorator::getInteractMsg() const {
  return Comp->getInteractMsg();
}

void Decorator::draw(cxxg::Screen &Scr) const { Comp->draw(Scr); }

void ExitHandlerProvider::registerExitHandler(ExitHandlerCallback EMC) {
  this->EMC = std::move(EMC);
}

void ExitHandlerProvider::handleExit() { EMC(); }

MoveDecorator::MoveDecorator(const std::shared_ptr<Widget> &Comp)
    : Decorator(Comp->getPos(), Comp) {}

bool MoveDecorator::handleInput(int Char) {
  switch (Char) {
  case cxxg::utils::KEY_ESC:
    handleExit();
    break;
  case cxxg::utils::KEY_LEFT:
    setPos(Pos + cxxg::types::Position{-1, 0});
    break;
  case cxxg::utils::KEY_RIGHT:
    setPos(Pos + cxxg::types::Position{1, 0});
    break;
  case cxxg::utils::KEY_UP:
    setPos(Pos + cxxg::types::Position{0, -1});
    break;
  case cxxg::utils::KEY_DOWN:
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