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

const std::vector<CraftingRecipe> &CraftingDatabase::getRecipes() const {
  return Recipes;
}

void CraftingDatabase::addRecipe(const CraftingRecipe &Recipe) {
  Recipes.push_back(Recipe);
}

} // namespace rogue