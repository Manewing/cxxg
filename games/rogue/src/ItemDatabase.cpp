#include <rogue/ItemDatabase.h>

namespace rogue {

static void fillItemDatabase(ItemDatabase &DB) {
  const int MaxStackSize = 99;

  auto HealEffectx10 = std::make_shared<HealItemEffect>(10);
  auto DamageEffectx10 = std::make_shared<DamageItemEffect>(10);

  std::vector<ItemPrototype> ItemProtos = {
      ItemPrototype(0, "Berry", ItemType::CONSUMABLE, MaxStackSize,
                    {HealEffectx10}),
      ItemPrototype(1, "Poison Berry", ItemType::CONSUMABLE, MaxStackSize,
                    {DamageEffectx10}),
      ItemPrototype(2, "Wood", ItemType::CRAFTING, MaxStackSize, {}),

      ItemPrototype(3, "Stone", ItemType::CRAFTING, MaxStackSize, {}),

  };

  for (const auto &Proto: ItemProtos) {
    DB.addItemProto(Proto.ItemId, Proto);
  }
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

}