#include <rogue/Inventory.h>

namespace rogue {

// FIXME move in and we "consume"?
void Inventory::addItem(const Item &It) {
  for (auto &InvIt : Items) {
    if (InvIt.isSameKind(It)) {
      // FIXME respespect max stack size
      InvIt.StackSize += It.StackSize;
      return;
    }
  }
  Items.push_back(It);
}

Item Inventory::takeItem(std::size_t ItemIdx) {
  // FIXME move item out?
  Item It = Items.at(ItemIdx);
  Items.erase(Items.begin() + ItemIdx);
  return It;
}

bool Inventory::canUseItem(const entt::entity &Entity, entt::registry &Reg,
                           std::size_t ItemIdx) {
  return Items.at(ItemIdx).canUseOn(Entity, Reg);
}

void Inventory::useItem(const entt::entity &Entity, entt::registry &Reg,
                        std::size_t ItemIdx, int NumItems) {
  auto &Itm = Items.at(ItemIdx);
  if (NumItems > Itm.StackSize) {
    return;
  }
  Itm.StackSize -= NumItems;

  for (int Cnt = 0; Cnt < NumItems; Cnt++) {
    Itm.useOn(Entity, Reg);
  }

  if (Itm.StackSize == 0) {
    Items.erase(Items.begin() + ItemIdx);
    return;
  }
}

bool Inventory::canEquipItemOn(const entt::entity &Entity, entt::registry &Reg,
                               std::size_t ItemIdx) {
  return Items.at(ItemIdx).canEquipOn(Entity, Reg);
}

void Inventory::equipItemOn(const entt::entity &Entity, entt::registry &Reg,
                            std::size_t ItemIdx) {
  Items.at(ItemIdx).equipOn(Entity, Reg);
}

bool Inventory::canUnequipItemFrom(const entt::entity &Entity,
                                   entt::registry &Reg, std::size_t ItemIdx) {
  return Items.at(ItemIdx).canUnequipFrom(Entity, Reg);
}

void Inventory::unequipItemFrom(const entt::entity &Entity, entt::registry &Reg,
                                std::size_t ItemIdx) {

  Items.at(ItemIdx).unequipFrom(Entity, Reg);
}

bool Inventory::empty() const { return Items.empty(); }

} // namespace rogue