#ifndef ROGUE_CRAFTING_HANDLER_H
#define ROGUE_CRAFTING_HANDLER_H

#include <optional>
#include <rogue/Item.h>
#include <vector>

namespace rogue {
class ItemDatabase;
}

namespace rogue {

class CraftingRecipe {
public:
  using ItemId = int;

public:
  CraftingRecipe(std::vector<ItemId> RequiredItems,
                 std::vector<ItemId> ResultItems)
      : RequiredItems(std::move(RequiredItems)), ResultItems(ResultItems) {}

  const std::vector<ItemId> &getRequiredItems() const { return RequiredItems; }
  const std::vector<ItemId> &getResultItems() const { return ResultItems; }

private:
  std::vector<ItemId> RequiredItems;
  std::vector<ItemId> ResultItems;
};

struct CraftingNode {
  using ItemId = int;

  std::optional<std::vector<ItemId>> ResultItems;
  std::map<ItemId, CraftingNode> Children;

  const std::optional<std::vector<ItemId>> &search(const std::vector<Item> &Items) const;
};

class CraftingHandler {
public:
  CraftingHandler() = default;
  explicit CraftingHandler(const ItemDatabase &ItemDb);
  void addRecipe(const CraftingRecipe &Recipe);

  std::optional<std::vector<Item>> tryCraft(const std::vector<Item> &Items) const;
  Item craftEnhancedItem(const std::vector<Item> &Items) const;

private:
  const ItemDatabase *ItemDb = nullptr;
  CraftingNode Tree;
};

} // namespace rogue

#endif // #ifndef ROGUE_CRAFTING_HANDLER_H