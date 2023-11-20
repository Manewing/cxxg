#ifndef ROGUE_ITEM_DATABASE_H
#define ROGUE_ITEM_DATABASE_H

#include <filesystem>
#include <map>
#include <rogue/Item.h>
#include <rogue/ItemPrototype.h>
#include <rogue/ItemSpecialization.h>
#include <rogue/LootTable.h>

namespace rogue {

class ItemDatabase {
public:
  static ItemDatabase load(const std::filesystem::path &ItemDbConfig);

public:
  int getNewItemId();
  int getItemId(const std::string &ItemName) const;

  const ItemPrototype &getItemProto(int ItemId) const;
  const ItemSpecializations *getItemSpec(int ItemId) const;

  void addItemProto(const ItemPrototype &ItemProto,
                    const ItemSpecializations *ItemSpec = nullptr);

  Item createItem(int ItemId, int StackSize = 1) const;

  int getRandomItemId() const;

  LootTable &addLootTable(const std::string &Name);
  const std::shared_ptr<LootTable> &getLootTable(const std::string &Name) const;

private:
  int MaxItemId = 0;
  std::map<std::string, int> ItemIdsByName;
  // FIXME make this a vector Id is index
  std::map<int, ItemPrototype> ItemProtos;
  std::map<int, ItemSpecializations> ItemSpecs;

  /// Map of loot table name to loot table
  std::map<std::string, std::shared_ptr<LootTable>> LootTables;
};

} // namespace rogue

#endif // #ifndef ROGUE_ITEM_DATABASE_H