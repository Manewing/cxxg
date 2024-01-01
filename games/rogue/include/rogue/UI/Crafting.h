#ifndef ROGUE_UI_CRAFTING_H
#define ROGUE_UI_CRAFTING_H

#include <entt/entt.hpp>
#include <rogue/UI/Decorator.h>

namespace rogue {
class CraftingDatabase;
class CraftingHandler;
class CraftingRecipe;
class Level;
} // namespace rogue

namespace rogue::ui {
class Controller;
class ListSelect;
} // namespace rogue::ui

namespace rogue::ui {

class CraftingController : public BaseRectDecorator {
public:
  CraftingController(Controller &Ctrl, entt::entity Entity, entt::registry &Reg,
                     const CraftingDatabase &CraftingDb,
                     const CraftingHandler &Crafter);
  bool handleInput(int Char) override;
  void draw(cxxg::Screen &Scr) const final;

  std::string getInteractMsg() const final;

private:
  const CraftingRecipe &getSelectedRecipe() const;
  void handleCreateTooltip();
  void handleCraft();
  void updateElements() const;

private:
  Controller &Ctrl;
  entt::entity Entity;
  entt::registry &Reg;
  const CraftingDatabase &CraftingDb;
  const CraftingHandler &Crafter;
  std::shared_ptr<ListSelect> List;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_CRAFTING_H