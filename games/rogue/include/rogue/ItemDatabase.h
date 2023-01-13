#ifndef ROGUE_ITEM_DATABASE_H
#define ROGUE_ITEM_DATABASE_H

#include <rogue/Item.h>
#include <map>

namespace rogue {

class ItemDatabase {
public:
  ItemDatabase();

  void addItemProto(int ItemId, const ItemPrototype &ItemProto);

  Item createItem(int ItemId, int StackSize = 1) const;

  std::map<int, ItemPrototype> ItemProtos;
};

}

#endif // #ifndef ROGUE_ITEM_DATABASE_H