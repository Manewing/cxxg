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
  static ItemDatabase load(const std::filesystem::path &ItemDbConfig,
                           const std::filesystem::path *SchemaPath = nullptr);

public:
  int getNewItemId();
  int getItemId(const std::string &ItemName) const;

  const std::map<int, ItemPrototype> &getItemProtos() const;

  const ItemPrototype &getItemProto(int ItemId) const;
  const ItemSpecializations *getItemSpec(int ItemId) const;

  void addItemProto(const ItemPrototype &ItemProto,
                    const ItemSpecializations *ItemSpec = nullptr,
                    const std::shared_ptr<LootTable> &Enhancements = nullptr);

  Item createItem(int ItemId, int StackSize = 1,
                  bool AllowEnchanting = true) const;

  int getRandomItemId() const;

  LootTable &addLootTable(const std::string &Name);
  const std::shared_ptr<LootTable> &getLootTable(const std::string &Name) const;
  const std::map<std::string, std::shared_ptr<LootTable>> &
  getLootTables() const;

  const std::shared_ptr<ItemEffect> &getItemEffect(const std::string &Name) const;

private:
  int MaxItemId = 0;
  std::map<std::string, int> ItemIdsByName;

  // FIXME make this a vector Id is index
  std::map<int, ItemPrototype> ItemProtos;
  std::map<int, ItemSpecializations> ItemSpecs;
  std::map<int, std::shared_ptr<LootTable>> ItemEnhancements;

  /// Map of loot table name to loot table
  std::map<std::string, std::shared_ptr<LootTable>> LootTables;

  /// Map of item effect name to item effect
  std::map<std::string, std::shared_ptr<ItemEffect>> Effects;
};

} // namespace rogue

#endif // #ifndef ROGUE_ITEM_DATABASE_H