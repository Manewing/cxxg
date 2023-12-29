#include <memory>
#include <rogue/CraftingDatabase.h>
#include <rogue/ItemDatabase.h>
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