#ifndef ROGUE_CRAFTING_DATABASE_H
#define ROGUE_CRAFTING_DATABASE_H

#include <filesystem>
#include <vector>
#include <map>

namespace rogue {
class ItemDatabase;
}

namespace rogue {

using CraftingRecipeId = int;

class CraftingRecipe {
public:
  using ItemId = int;

public:
  CraftingRecipe(std::string Name, std::vector<ItemId> RequiredItems,
                 std::vector<ItemId> ResultItems)
      : Name(std::move(Name)), RequiredItems(std::move(RequiredItems)),
        ResultItems(std::move(ResultItems)) {}

  const std::string &getName() const { return Name; }
  const std::vector<ItemId> &getRequiredItems() const { return RequiredItems; }
  const std::vector<ItemId> &getResultItems() const { return ResultItems; }

private:
  std::string Name;
  std::vector<ItemId> RequiredItems;
  std::vector<ItemId> ResultItems;
};

class CraftingDatabase {
public:
  static CraftingDatabase load(const ItemDatabase &ItemDb,
                               const std::filesystem::path &CraftingDbConfig);

public:
  CraftingDatabase() = default;
  explicit CraftingDatabase(std::map<CraftingRecipeId, CraftingRecipe> Recipes)
      : Recipes(std::move(Recipes)) {}

  CraftingRecipeId getRecipeId(const std::string &Name) const;
  const CraftingRecipe &getRecipe(CraftingRecipeId Id) const;

  const std::map<CraftingRecipeId, CraftingRecipe> &getRecipes() const;
  void addRecipe(const CraftingRecipe &Recipe);

private:
  std::map<std::string, CraftingRecipeId> RecipeNameToId;
  std::map<CraftingRecipeId, CraftingRecipe> Recipes;
};

} // namespace rogue

#endif // #ifndef ROGUE_CRAFTING_DATABASE_H