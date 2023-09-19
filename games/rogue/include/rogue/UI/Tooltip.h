#ifndef ROGUE_UI_ITEM_TOOLTIP_H
#define ROGUE_UI_ITEM_TOOLTIP_H

#include "rogue/UI/Decorator.h"
#include <rogue/Item.h>
#include <rogue/UI/Widget.h>

namespace rogue::ui {

class Tooltip : public Decorator {
public:
  Tooltip(cxxg::types::Position Pos, cxxg::types::Size Size,
          std::string Text, std::string Header = "");
};

class ItemTooltip : public  Tooltip {
public:
  ItemTooltip(cxxg::types::Position Pos, cxxg::types::Size Size,const Item &It);
};


}

#endif // #define ROGUE_UI_ITEM_TOOLTIP_H