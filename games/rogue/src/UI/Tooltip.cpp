#include <memory>
#include <rogue/UI/Controls.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/Item.h>
#include <rogue/UI/TextBox.h>
#include <rogue/UI/Tooltip.h>
#include <sstream>

namespace rogue::ui {

Tooltip::Tooltip(cxxg::types::Position Pos, cxxg::types::Size Size,
                 std::string Text, std::string Header)
    : Decorator(Pos, nullptr) {
  Comp = std::make_shared<TextBox>(Pos, Size, Text);
  Comp = std::make_shared<Frame>(Comp, Pos, Size, Header);
}

bool Tooltip::handleInput(int Char) {
  if (Char == Controls::Info.Char) {
    return false;
  }
  return Decorator::handleInput(Char);
}

ItemTooltip::ItemTooltip(cxxg::types::Position Pos, cxxg::types::Size Size,
                         const Item &It, bool Equipped)
    : Tooltip(Pos, Size, getItemText(It),
              (Equipped ? "Equip: " : "") + It.getName()) {
  static_cast<Frame *>(Comp.get())->setHeaderColor(getColorForItem(It));
}

} // namespace rogue::ui