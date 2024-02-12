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

/// Crafting recipes are stored in a tree-like structure. Each node stores a
/// mapping of item Id that is required to craft the results defined in
/// subsequent nodes.
///
/// For example:
///
///   {Root}
///    |
///    +-- Wood -> {Node 1}
///    |           |
///    |           +-- Iron  -> {Node 2}
///    |           |            +-{Results: Iron Axe}
///    |           |
///    |           +-- Stone -> {Node 3}
///    |                         +-{Results: Stone axe}
///    +-- Iron -> {Node 4}      |
///                |             +- Rune Stone -> {Node 5}
///                |                              +-{Results: Rune Axe}
///                |
///                +-- Stone -> {Node 6}
///                              +-{Results: Flint and Steel}
struct CraftingNode {
  struct CraftingResult {
    CraftingRecipeId RecipeId;
    std::vector<ItemProtoId> Items;
  };

  /// Each node may define a result, which is a list of items that are crafted
  /// when the node is reached
  std::optional<CraftingResult> Result;

  /// Children of the nodes, key is the item Id required to continue to child
  /// node.
  std::map<ItemProtoId, CraftingNode> Children;

  const std::optional<CraftingResult> &
  search(const std::vector<Item> &Items) const;
};

class CraftingHandler {
public:
  CraftingHandler() = default;
  explicit CraftingHandler(const ItemDatabase &ItemDb);
  void addRecipe(CraftingRecipeId RecipeId, const CraftingRecipe &Recipe);

  const ItemDatabase *getItemDb() const;
  const ItemDatabase &getItemDbOrFail() const;

  const std::optional<CraftingNode::CraftingResult> &
  getCraftingRecipeResultOrNone(const std::vector<Item> &Items) const;

  std::optional<std::vector<Item>>
  tryCraft(const std::vector<Item> &Items) const;

  std::optional<std::vector<Item>>
  tryCraftAsRecipe(const std::vector<Item> &Items) const;
  Item craftEnhancedItem(const std::vector<Item> &Items) const;

private:
  const ItemDatabase *ItemDb = nullptr;
  CraftingNode Tree;
};

} // namespace rogue

#endif // #ifndef ROGUE_CRAFTING_HANDLER_H