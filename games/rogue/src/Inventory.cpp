#include <iostream>
#include <rogue/Inventory.h>

namespace rogue {

bool Inventory::applyItemTo(const Item &It, CapabilityFlags Flags,
                            entt::entity SrcEt, entt::entity DstEt,
                            entt::registry &Reg) {
  if (!It.canApplyTo(SrcEt, DstEt, Reg, Flags)) {
    return false;
  }
  It.applyTo(SrcEt, DstEt, Reg, Flags);
  return true;
}

void Inventory::addItem(Item It) {
  for (auto &InvIt : Items) {
    if (InvIt.isSameKind(It) && InvIt.StackSize < It.getMaxStackSize() &&
        (MaxStackSize == 0 ||
         static_cast<unsigned>(InvIt.StackSize) < MaxStackSize)) {
      auto Stacks =
          std::min(It.getMaxStackSize() - InvIt.StackSize, It.StackSize);
      if (MaxStackSize > 0) {
        Stacks = std::min(static_cast<unsigned>(Stacks), MaxStackSize);
      }
      InvIt.StackSize += Stacks;
      It.StackSize -= Stacks;

      if (It.StackSize == 0) {
        return;
      }
    }
  }

  auto InvMaxSize = MaxStackSize;
  if (MaxStackSize == 0) {
    InvMaxSize = std::numeric_limits<unsigned>::max();
  }

  auto RemainingStacks = It.StackSize;
  while (RemainingStacks > 0) {
    auto Stacks = std::min(static_cast<unsigned>(RemainingStacks), InvMaxSize);
    Stacks = std::min(Stacks, static_cast<unsigned>(It.getMaxStackSize()));
    It.StackSize = Stacks;
    Items.push_back(It);
    RemainingStacks -= Stacks;
  }
}

Item Inventory::takeItem(std::size_t ItemIdx) {
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

bool Inventory::hasItem(int Id, unsigned Count) const {
  if (Count == 0) {
    return false;
  }
  unsigned FoundCount = 0;
  for (const auto &It : Items) {
    if (It.getId() == Id) {
      FoundCount += It.StackSize;
      if (FoundCount >= Count) {
        return true;
      }
    }
  }
  return false;
}

std::optional<std::size_t> Inventory::getItemIndexForId(int Id) const {
  for (std::size_t Idx = 0; Idx < Items.size(); ++Idx) {
    if (Items.at(Idx).getId() == Id) {
      return Idx;
    }
  }
  return {};
}

std::optional<Item> Inventory::applyItemTo(std::size_t ItemIdx,
                                           CapabilityFlags Flags,
                                           entt::entity SrcEt,
                                           entt::entity DstEt,
                                           entt::registry &Reg) {
  if (applyItemTo(getItem(ItemIdx), Flags, SrcEt, DstEt, Reg)) {
    return takeItem(ItemIdx, /*Count=*/1);
  }
  return {};
}

std::size_t Inventory::size() const { return Items.size(); }

bool Inventory::empty() const { return Items.empty(); }

void Inventory::clear() { Items.clear(); }

std::ostream &operator<<(std::ostream &OS, const Inventory &Inv) {
  OS << "Inventory:\n";
  for (const auto &It : Inv.getItems()) {
    OS << " -> " << It.getName() << " x" << It.StackSize << "\n";
  }
  return OS;
}

} // namespace rogue