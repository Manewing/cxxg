#ifndef ROGUE_CRAFTING_DATABASE_H
#define ROGUE_CRAFTING_DATABASE_H

#include <filesystem>
#include <vector>

namespace rogue {
class ItemDatabase;
}

namespace rogue {

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
  explicit CraftingDatabase(std::vector<CraftingRecipe> Recipes)
      : Recipes(std::move(Recipes)) {}

  const std::vector<CraftingRecipe> &getRecipes() const;
  void addRecipe(const CraftingRecipe &Recipe);

private:
  std::vector<CraftingRecipe> Recipes;
};

} // namespace rogue

#endif // #ifndef ROGUE_CRAFTING_DATABASE_H