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

Item Inventory::takeItem(std::size_t ItemIdx, unsigned Count) {
  Item &It = Items.at(ItemIdx);
  if (It.StackSize > static_cast<int>(Count)) {
    It.StackSize -= Count;
    auto SubStack = It;
    SubStack.StackSize = Count;
    return SubStack;
  }
  // Count exceeds stack size, just return everything
  return takeItem(ItemIdx);
}

std::optional<Item> Inventory::applyItemTo(std::size_t ItemIdx,
                                           CapabilityFlags Flags,
                                           entt::entity Entity,
                                           entt::registry &Reg) {
  if (!getItem(ItemIdx).canApplyTo(Entity, Reg, Flags)) {
    // FIXME message
    return {};
  }
  auto It = takeItem(ItemIdx, /*Count=*/1);
  It.applyTo(Entity, Reg, Flags);
  return It;
}

std::size_t Inventory::size() const { return Items.size(); }

bool Inventory::empty() const { return Items.empty(); }

} // namespace rogue