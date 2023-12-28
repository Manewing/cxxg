#include <rogue/CraftingDatabase.h>
#include <rogue/CraftingHandler.h>
#include <rogue/Event.h>
#include <rogue/InventoryHandler.h>
#include <rogue/UI/Controller.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/Crafting.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/ListSelect.h>
#include <rogue/UI/Tooltip.h>
#include <rogue/Components/Player.h>

namespace rogue::ui {

constexpr cxxg::types::Size TooltipSize = {40, 10};
constexpr cxxg::types::Position TooltipOffset = {4, 4};

CraftingController::CraftingController(Controller &Ctrl, entt::entity Entity,
                                       entt::registry &Reg,
                                       const CraftingDatabase &CraftingDb,
                                       const CraftingHandler &Crafter)
    : BaseRectDecorator({2, 2}, {40, 18}, nullptr), Ctrl(Ctrl), Entity(Entity),
      Reg(Reg), CraftingDb(CraftingDb), Crafter(Crafter) {
  List = std::make_shared<ListSelect>(Pos, getSize());
  Comp = std::make_shared<Frame>(List, Pos, getSize(), "Crafting");
  updateElements();
}

bool CraftingController::handleInput(int Char) {
  switch (Char) {
  case Controls::Info.Char: {
    handleCreateTooltip();
  } break;
  case Controls::Craft.Char: {
    handleCraft();
  } break;
  case Controls::CloseWindow.Char:
    return false;
  default:
    return List->handleInput(Char);
  }
  return true;
}

void CraftingController::draw(cxxg::Screen &Scr) const {
  updateElements();
  BaseRectDecorator::draw(Scr);
}

std::string CraftingController::getInteractMsg() const {
  std::vector<KeyOption> Options = {Controls::Navigate, Controls::Info,
                                    Controls::Craft};
  return KeyOption::getInteractMsg(Options);
}

const CraftingRecipe &CraftingController::getSelectedRecipe() const {
  const auto SelIdx = List->getSelectedElement();
  auto &Name = List->getElements().at(SelIdx).Text;
  const auto RecipeId = CraftingDb.getRecipeId(Name);
  return CraftingDb.getRecipe(RecipeId);
}

void CraftingController::handleCreateTooltip() {
  const auto &Recipe = getSelectedRecipe();
  InventoryHandler InvHandler(Entity, Reg, Crafter);
  Ctrl.addWindow(std::make_shared<CraftingRecipeTooltip>(
      Pos + TooltipOffset, TooltipSize, Recipe, InvHandler.canCraft(Recipe),
      Crafter.getItemDbOrFail()));
}

void CraftingController::handleCraft() {
  const auto &Recipe = getSelectedRecipe();
  InventoryHandler InvHandler(Entity, Reg, Crafter);

  if (!InvHandler.canCraft(Recipe)) {
    Ctrl.publish(PlayerInfoMessageEvent()
                 << "You do not have the required items to craft this.");
    return;
  }

  if (!InvHandler.tryCraft(Recipe)) {
    Ctrl.publish(ErrorMessageEvent() << "Unexpectedly failed to craft item.");
  } else {
    Ctrl.publish(PlayerInfoMessageEvent()
                 << "You crafted " << Recipe.getName() << ".");
  }
}

void CraftingController::updateElements() const {
  const auto &Recipes = CraftingDb.getRecipes();

  auto *PC = Reg.try_get<PlayerComp>(Entity);
  if (!PC) {
    Ctrl.publish(ErrorMessageEvent()
                 << "Failed to get PlayerComp for crafting controller.");
    return;
  }

  std::vector<ListSelect::Element> Elements;
  Elements.reserve(PC->KnownRecipes.size());
  InventoryHandler InvHandler(Entity, Reg, Crafter);
  for (const auto &RecipeId : PC->KnownRecipes) {
    const auto &Recipe = Recipes.at(RecipeId);
    auto Color = cxxg::types::RgbColor{200, 80, 55};
    if (InvHandler.canCraft(Recipe)) {
      Color = cxxg::types::RgbColor{40, 130, 40};
    }
    Elements.push_back({Recipe.getName(), Color});
  }
  auto PrevIdx = List->getSelectedElement();
  List->setElements(Elements);
  List->selectElement(PrevIdx);
}

} // namespace rogue::ui