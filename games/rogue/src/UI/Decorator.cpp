#include <rogue/UI/Decorator.h>

namespace rogue::ui {

Decorator::Decorator(std::shared_ptr<Widget> Comp) : Comp(std::move(Comp)) {}

bool Decorator::handleInput(int Char) { return Comp->handleInput(Char); }

std::string_view Decorator::getInteractMsg() const {
  return Comp->getInteractMsg();
}

void Decorator::draw(cxxg::Screen &Scr) const { Comp->draw(Scr); }

} // namespace rogue::ui