#include <memory>
#include <rogue/CraftingDatabase.h>
#include <rogue/ItemDatabase.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/Item.h>
#include <rogue/UI/ItemSelect.h>
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

YesNoDialog::YesNoDialog(cxxg::types::Position Pos, cxxg::types::Size Size,
                         std::string Text, const std::function<void(bool)> &Cb)
    : BaseRectDecorator(Pos, Size, nullptr) {
  cxxg::types::Size TextBoxSize{Size.X - 4, Size.Y - 4};
  TB = std::make_shared<TextBox>(Pos, TextBoxSize, Text, cxxg::types::Size{0, 0});

  cxxg::types::Position ItSelPos{Pos.X,
                                 Pos.Y + static_cast<int>(TextBoxSize.Y)};
  auto ItSel = std::make_shared<ItemSelect>(ItSelPos);
  ItSel->addSelect<Select>("Yes", ItSelPos + cxxg::types::Position{2, 0}, 3);
  ItSel->addSelect<Select>("No", ItSelPos + cxxg::types::Position{6, 0}, 2);

  ItSel->registerOnSelectCallback([Cb](const Select &Sel) {
    if (Sel.getValue() == "Yes") {
      Cb(true);
    } else {
      Cb(false);
    }
  });

  Comp = std::make_shared<Frame>(ItSel, Pos, Size, "Confirm");

  setPos(Pos);
}

bool YesNoDialog::handleInput(int Char) {
  switch (Char) {
  case Controls::MoveUp.Char:
  case Controls::MoveDown.Char:
    return TB->handleInput(Char);
  case Controls::MoveLeft.Char:
    Char = Controls::MoveUp.Char;
    break;
  case Controls::MoveRight.Char:
    Char = Controls::MoveDown.Char;
    break;
  case Controls::Interact.Char:
    BaseRectDecorator::handleInput(Char);
    return false;
  case Controls::Info.Char:
    return false;
  default:
    break;
  }
  return BaseRectDecorator::handleInput(Char);
}

void YesNoDialog::setPos(cxxg::types::Position P) {
  BaseRectDecorator::setPos(P);
  TB->setPos(cxxg::types::Position{P.X + 2, P.Y + 1});
}

void YesNoDialog::draw(cxxg::Screen &Scr) const {
  BaseRectDecorator::draw(Scr);
  TB->draw(Scr);
}

ItemTooltip::ItemTooltip(cxxg::types::Position Pos, cxxg::types::Size Size,
                         const Item &It, bool Equipped)
    : Tooltip(Pos, Size, getItemText(It),
              (Equipped ? "Equip: " : "") + It.getQualifierName()) {
  static_cast<Frame *>(Comp.get())->setHeaderColor(getColorForItem(It));
}

namespace {

std::string getRecipeText(const CraftingRecipe &Recipe, bool CanCraft,
                          const ItemDatabase &ItemDb) {
  std::stringstream SS;
  SS << "Ingredients:\n";
  for (const auto &ItId : Recipe.getRequiredItems()) {
    SS << " - " << ItemDb.getItemProto(ItId).Name << "\n";
  }
  SS << "\n";
  SS << "Results:\n";
  for (const auto &ItId : Recipe.getResultItems()) {
    SS << " - " << ItemDb.getItemProto(ItId).Name << "\n";
  }
  SS << "\n---\n\n";
  if (CanCraft) {
    SS << "You have the required ingredients\n";
  } else {
    SS << "You don't have the required ingredients\n";
  }
  return SS.str();
}

} // namespace

CraftingRecipeTooltip::CraftingRecipeTooltip(cxxg::types::Position Pos,
                                             cxxg::types::Size Size,
                                             const CraftingRecipe &Recipe,
                                             bool CanCraft,
                                             const ItemDatabase &ItemDb)
    : Tooltip(Pos, Size, getRecipeText(Recipe, CanCraft, ItemDb),
              Recipe.getName()) {}

} // namespace rogue::ui