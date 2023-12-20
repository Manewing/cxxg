#ifndef ROGUE_CRAFTING_HANDLER_H
#define ROGUE_CRAFTING_HANDLER_H

#include <optional>
#include <rogue/CraftingDatabase.h>
#include <rogue/Item.h>
#include <vector>

namespace rogue {
class ItemDatabase;
}

namespace rogue {

struct CraftingNode {
  using ItemId = int;

  std::optional<std::vector<ItemId>> ResultItems;
  std::map<ItemId, CraftingNode> Children;

  const std::optional<std::vector<ItemId>> &
  search(const std::vector<Item> &Items) const;
};

class CraftingHandler {
public:
  CraftingHandler() = default;
  explicit CraftingHandler(const ItemDatabase &ItemDb);
  void addRecipe(const CraftingRecipe &Recipe);

  std::optional<std::vector<Item>>
  tryCraft(const std::vector<Item> &Items) const;
  Item craftEnhancedItem(const std::vector<Item> &Items) const;

private:
  const ItemDatabase *ItemDb = nullptr;
  CraftingNode Tree;
};

} // namespace rogue

#endif // #ifndef ROGUE_CRAFTING_HANDLER_H