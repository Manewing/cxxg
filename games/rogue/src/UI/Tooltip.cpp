#include "rogue/UI/Frame.h"
#include <memory>
#include <rogue/UI/TextBox.h>
#include <rogue/UI/Tooltip.h>

namespace rogue::ui {

Tooltip::Tooltip(cxxg::types::Position Pos, cxxg::types::Size Size,
                 std::string Text, std::string Header)
    : Decorator(Pos, nullptr) {
  Comp = std::make_shared<TextBox>(Pos, Size, Text);
  Comp = std::make_shared<Frame>(Comp, Pos, Size, Header);
}

ItemTooltip::ItemTooltip(cxxg::types::Position Pos, cxxg::types::Size Size,
                         const Item &It)
    : Tooltip(Pos, Size, It.getDescription(), It.getName()) {}

} // namespace rogue::ui