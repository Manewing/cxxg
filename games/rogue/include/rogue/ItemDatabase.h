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
  ItemProtoId getNewItemId();
  ItemProtoId getItemId(const std::string &ItemName) const;

  const std::map<ItemProtoId, ItemPrototype> &getItemProtos() const;

  const ItemPrototype &getItemProto(ItemProtoId ItemId) const;
  const ItemSpecializations *getItemSpec(ItemProtoId ItemId) const;

  void addItemProto(const ItemPrototype &ItemProto,
                    const ItemSpecializations *ItemSpec = nullptr,
                    const std::shared_ptr<LootTable> &Enhancements = nullptr);

  Item createItem(ItemProtoId ItemId, int StackSize = 1,
                  bool AllowEnchanting = true) const;

  ItemProtoId getRandomItemId() const;

  LootTable &addLootTable(const std::string &Name);
  const std::shared_ptr<LootTable> &getLootTable(const std::string &Name) const;
  const std::map<std::string, std::shared_ptr<LootTable>> &
  getLootTables() const;

  const std::shared_ptr<ItemEffect> &getItemEffect(const std::string &Name) const;

private:
  int MaxItemId = 0;
  std::map<std::string, ItemProtoId> ItemIdsByName;

  // FIXME make this a vector Id is index
  std::map<ItemProtoId, ItemPrototype> ItemProtos;
  std::map<ItemProtoId, ItemSpecializations> ItemSpecs;
  std::map<ItemProtoId, std::shared_ptr<LootTable>> ItemEnhancements;

  /// Map of loot table name to loot table
  std::map<std::string, std::shared_ptr<LootTable>> LootTables;

  /// Map of item effect name to item effect
  std::map<std::string, std::shared_ptr<ItemEffect>> Effects;
};

} // namespace rogue

#endif // #ifndef ROGUE_ITEM_DATABASE_H