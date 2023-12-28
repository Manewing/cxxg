#include <rogue/CraftingDatabase.h>
#include <rogue/ItemDatabase.h>
#include <rogue/JSON.h>

namespace rogue {

CraftingDatabase
CraftingDatabase::load(const ItemDatabase &ItemDb,
                       const std::filesystem::path &CraftingDbConfig) {
  CraftingDatabase CraftingDb;

  const auto SchemaPath =
      CraftingDbConfig.parent_path() / "schemas" / "crafting_db_schema.json";
  auto [DocStr, Doc] = loadJSON(CraftingDbConfig, &SchemaPath);

  for (const auto &Recipe : Doc["recipes"].GetArray()) {
    auto Name = Recipe["name"].GetString();

    std::vector<CraftingRecipe::ItemId> RequiredItems;
    for (const auto &RequiredItem : Recipe["ingredients"].GetArray()) {
      RequiredItems.push_back(ItemDb.getItemId(RequiredItem.GetString()));
    }

    std::vector<CraftingRecipe::ItemId> ResultItems;
    for (const auto &ResultItem : Recipe["results"].GetArray()) {
      ResultItems.push_back(ItemDb.getItemId(ResultItem.GetString()));
    }

    CraftingDb.addRecipe(
        CraftingRecipe(Name, std::move(RequiredItems), std::move(ResultItems)));
  }

  return CraftingDb;
}

CraftingRecipeId CraftingDatabase::getRecipeId(const std::string &Name) const {
  auto It = RecipeNameToId.find(Name);
  if (It == RecipeNameToId.end()) {
    throw std::runtime_error("CraftingDatabase::getRecipeId: Unknown recipe: " +
                             Name);
  }
  return It->second;
}

const CraftingRecipe &CraftingDatabase::getRecipe(CraftingRecipeId Id) const {
  auto It = Recipes.find(Id);
  if (It == Recipes.end()) {
    throw std::runtime_error(
        "CraftingDatabase::getRecipe: Unknown recipe id: " +
        std::to_string(Id));
  }
  return It->second;
}

const std::map<CraftingRecipeId, CraftingRecipe> &
CraftingDatabase::getRecipes() const {
  return Recipes;
}

void CraftingDatabase::addRecipe(const CraftingRecipe &Recipe) {
  if (Recipe.getRequiredItems().size() < 2) {
    throw std::runtime_error("CraftingDatabase::addRecipe: Invalid recipe");
  }
  if (RecipeNameToId.count(Recipe.getName()) != 0) {
    throw std::runtime_error(
        "CraftingDatabase::addRecipe: Duplicate recipe name: " +
        Recipe.getName());
  }
  auto RecipeId = Recipes.size();
  RecipeNameToId[Recipe.getName()] = RecipeId;
  Recipes.emplace(RecipeId, Recipe);
}

} // namespace rogue