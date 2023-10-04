#ifndef ROGUE_ITEM_DATABASE_H
#define ROGUE_ITEM_DATABASE_H

#include <filesystem>
#include <map>
#include <rogue/Item.h>
#include <rogue/ItemPrototype.h>
#include <rogue/ItemSpecialization.h>

namespace rogue {

class ItemDatabase {
public:
  static ItemDatabase load(const std::filesystem::path &ItemDbConfig);

public:
  int getItemId(const std::string &ItemName) const;

  void addItemProto(const ItemPrototype &ItemProto,
                    const ItemSpecializations *ItemSpec = nullptr);

  Item createItem(int ItemId, int StackSize = 1) const;

  int getRandomItemId() const;

private:
  std::map<std::string, int> ItemIdsByName;
  // FIXME make this a vector Id is index
  std::map<int, ItemPrototype> ItemProtos;
  std::map<int, ItemSpecializations> ItemSpecs;
};

} // namespace rogue

#endif // #ifndef ROGUE_ITEM_DATABASE_H