#ifndef ROGUE_CRAFTING_HANDLER_H
#define ROGUE_CRAFTING_HANDLER_H

#include <optional>
#include <rogue/Item.h>
#include <vector>

namespace rogue {
class ItemDatabase;
}

namespace rogue {

class CraftingHandler {
public:
  explicit CraftingHandler(ItemDatabase &ItemDb);
  std::optional<Item> tryCraft(const std::vector<Item> &Items);
  Item craftEnhancedItem(const std::vector<Item> &Items);

private:
  ItemDatabase &ItemDb;
};

} // namespace rogue

#endif // #ifndef ROGUE_CRAFTING_HANDLER_H