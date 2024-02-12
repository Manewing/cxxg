#ifndef ROGUE_CRAFTING_DATABASE_H
#define ROGUE_CRAFTING_DATABASE_H

#include <filesystem>
#include <map>
#include <vector>
#include <rogue/Types.h>

namespace rogue {
class ItemDatabase;
}

namespace rogue {

class CraftingRecipe {
public:
  CraftingRecipe(std::string Name, std::vector<ItemProtoId> RequiredItems,
                 std::vector<ItemProtoId> ResultItems)
      : Name(std::move(Name)), RequiredItems(std::move(RequiredItems)),
        ResultItems(std::move(ResultItems)) {}

  const std::string &getName() const { return Name; }
  const std::vector<ItemProtoId> &getRequiredItems() const { return RequiredItems; }
  const std::vector<ItemProtoId> &getResultItems() const { return ResultItems; }

private:
  std::string Name;
  std::vector<ItemProtoId> RequiredItems;
  std::vector<ItemProtoId> ResultItems;
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