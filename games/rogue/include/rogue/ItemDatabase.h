#ifndef ROGUE_ITEM_DATABASE_H
#define ROGUE_ITEM_DATABASE_H

#include <filesystem>
#include <map>
#include <rogue/Item.h>

namespace rogue {

class ItemDatabase {
public:
  static ItemType getItemType(const std::string &ItemTypeStr);
  static CapabilityFlags getCapabilityFlag(const std::string &CapabilityFlagStr);

  static ItemDatabase load(const std::filesystem::path &ItemDbConfig);

public:
  void addItemProto(const ItemPrototype &ItemProto);

  Item createItem(int ItemId, int StackSize = 1) const;

  std::map<int, ItemPrototype> ItemProtos;
};

} // namespace rogue

#endif // #ifndef ROGUE_ITEM_DATABASE_H