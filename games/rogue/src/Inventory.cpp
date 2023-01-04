#include "Inventory.h"

void Inventory::addItem(const Item &It) {
  for (auto &InvIt : Items) {
    if (&InvIt.getProto() == &It.getProto()) {
      InvIt.StackSize += It.StackSize;
      return;
    }
  }
  Items.push_back(It);
}

bool Inventory::canUseItem(const entt::entity &Entity, entt::registry &Reg,
                           std::size_t ItemIdx) {
  return Items.at(ItemIdx).getProto().canUseOn(Entity, Reg);
}

void Inventory::useItem(const entt::entity &Entity, entt::registry &Reg,
                        std::size_t ItemIdx, int NumItems) {
  auto &Itm = Items.at(ItemIdx);
  if (NumItems > Itm.StackSize) {
    return;
  }
  Itm.StackSize -= NumItems;

  Itm.getProto().useOn(Entity, Reg, NumItems);

  if (Itm.StackSize == 0) {
    Items.erase(Items.begin() + ItemIdx);
    return;
  }
}

bool Inventory::empty() const { return Items.empty(); }