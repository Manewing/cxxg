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

bool Inventory::empty() const { return Items.empty(); }

} // namespace rogue