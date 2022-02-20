#include "Inventory.h"
#include "Entity.h"

Inventory::Inventory(Entity &Parent) : Parent(Parent) {}

void Inventory::consumeItem(std::size_t ItemIdx, int NumItems) {
  auto &Itm = Items.at(ItemIdx);
  if (NumItems > Itm.StackSize) {
    return;
  }
  Itm.StackSize -= NumItems;

  for (const auto &Effect : Itm.getProto().Effects) {
    Effect->apply(Parent, NumItems);
  }

  if (Itm.StackSize == 0) {
    Items.erase(Items.begin() + ItemIdx);
    return;
  }
}

bool Inventory::empty() const { return Items.empty(); }