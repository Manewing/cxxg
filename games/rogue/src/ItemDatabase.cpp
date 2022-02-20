#include "ItemDatabase.h"

static void fillItemDatabase(ItemDatabase &DB) {
  const int MaxStackSize = 99;
  DB.addItemProto(
      0, ItemPrototype(0, "Berry", ItemType::CONSUMABLE, MaxStackSize));
  DB.addItemProto(
      1, ItemPrototype(1, "Wood", ItemType::CRAFTING, MaxStackSize));
  DB.addItemProto(
      2, ItemPrototype(2, "Stone", ItemType::CRAFTING, MaxStackSize));
}

ItemDatabase::ItemDatabase() { fillItemDatabase(*this); }

void ItemDatabase::addItemProto(int ItemId, const ItemPrototype &ItemProto) {
  // FIXME ID not yet taken
  ItemProtos.emplace(ItemId, ItemProto);
}

Item ItemDatabase::createItem(int ItemId, int StackSize) const {
  const auto &Proto = ItemProtos.at(ItemId);
  return Item(Proto, StackSize);
}