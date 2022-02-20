#include "Item.h"

ItemPrototype::ItemPrototype(int ItemId, std::string N, ItemType Type,
                             int MaxStatckSize)
    : ItemId(ItemId), Name(std::move(N)), Type(Type),
      MaxStatckSize(MaxStatckSize) {}

Item::Item(const ItemPrototype &Proto, int StackSize)
    : StackSize(StackSize), Proto(&Proto) {}

const ItemPrototype &Item::getProto() const { return *Proto; }