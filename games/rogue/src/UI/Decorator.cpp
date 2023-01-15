#include <rogue/UI/Decorator.h>

namespace rogue::ui {

Decorator::Decorator(cxxg::types::Position Pos, std::shared_ptr<Widget> Comp)
    : Widget(Pos), Comp(std::move(Comp)) {}

void Decorator::setPos(cxxg::types::Position P) {
  Pos = P;
  Comp->setPos(P);
}

bool Decorator::handleInput(int Char) { return Comp->handleInput(Char); }

std::string_view Decorator::getInteractMsg() const {
  return Comp->getInteractMsg();
}

void Decorator::draw(cxxg::Screen &Scr) const { Comp->draw(Scr); }

} // namespace rogue::ui