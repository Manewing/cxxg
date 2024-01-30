#ifndef ROGUE_UI_ITEM_TOOLTIP_H
#define ROGUE_UI_ITEM_TOOLTIP_H

#include <rogue/Item.h>
#include <rogue/UI/Decorator.h>
#include <rogue/UI/Widget.h>

namespace rogue {
class CraftingRecipe;
class ItemDatabase;
} // namespace rogue

namespace rogue::ui {

class Tooltip : public Decorator {
public:
  Tooltip(cxxg::types::Position Pos, cxxg::types::Size Size, std::string Text,
          std::string Header = "");
  bool handleInput(int Char) override;
};

class YesNoDialog : public BaseRectDecorator {
public:
  YesNoDialog(cxxg::types::Position Pos, cxxg::types::Size Size,
              std::string Text, const std::function<void(bool)> &Cb);
  bool handleInput(int Char) override;

  void setPos(cxxg::types::Position P) override;
  void draw(cxxg::Screen &Scr) const override;

private:
  std::shared_ptr<Widget> TB;
};

class ItemTooltip : public Tooltip {
public:
  ItemTooltip(cxxg::types::Position Pos, cxxg::types::Size Size, const Item &It,
              bool Equipped);
};

class CraftingRecipeTooltip : public Tooltip {
public:
  CraftingRecipeTooltip(cxxg::types::Position Pos, cxxg::types::Size Size,
                        const CraftingRecipe &Recipe, bool CanCraft,
                        const ItemDatabase &ItemDb);
};

} // namespace rogue::ui

#endif // #define ROGUE_UI_ITEM_TOOLTIP_H