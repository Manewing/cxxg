#include <algorithm>
#include <cassert>
#include <random>
#include <rogue/LootTable.h>
#include <stdexcept>

namespace rogue {

std::vector<LootContainer::LootReward> LootContainer::generateLoot() const {
  std::vector<LootContainer::LootReward> Loot;
  fillLoot(Loot);
  return Loot;
}

LootItem::LootItem(ItemId ItId, unsigned MinCount, unsigned MaxCount)
    : ItId(ItId), MinCount(MinCount), MaxCount(MaxCount) {
  assert(MinCount <= MaxCount && MinCount > 0);
}

LootContainer::ItemId LootItem::getItemId() const { return ItId; }

void LootItem::fillLoot(std::vector<LootReward> &Loot) const {
  auto Count = MinCount + std::rand() % (MaxCount - MinCount + 1);
  Loot.push_back({ItId, Count});
}

std::size_t LootTable::getSlotForRoll(int Roll,
                                      const std::vector<LootSlot> &Slots) {
  for (std::size_t Idx = 0; Idx < Slots.size(); ++Idx) {
    Roll -= Slots.at(Idx).Weight;
    if (Roll < 0) {
      return Idx;
    }
  }
  throw std::runtime_error("LootTable::getSlotForRoll() failed");
}

std::size_t LootTable::rollForSlot(const std::vector<LootSlot> &Slots) {
  // FIXME this needs to be based on the seed as well
  int TotalWeight = 0;
  for (const auto &Slot : Slots) {
    TotalWeight += Slot.Weight;
  }
  int Roll = std::rand() % TotalWeight;
  return getSlotForRoll(Roll, Slots);
}

LootTable::LootTable() { reset(0, {}); }

LootTable::LootTable(unsigned NumRolls, const std::vector<LootSlot> &Sls)
    : NumRolls(NumRolls) {
  reset(NumRolls, Sls);
}

void LootTable::reset(unsigned NR, const std::vector<LootSlot> &Sls) {
  GuaranteedSlots.clear();
  Slots.clear();

  NumRolls = NR;
  for (const auto &Slot : Sls) {
    if (Slot.Weight == -1) {
      GuaranteedSlots.push_back(Slot);
    } else {
      Slots.push_back(Slot);
    }
  }
  NumRolls = Slots.empty() ? 0 : NumRolls;

  std::sort(Slots.begin(), Slots.end(),
            [](const auto &A, const auto &B) { return A.Weight < B.Weight; });
}

const std::vector<LootTable::LootSlot> &LootTable::getSlots() const {
  return Slots;
}

const std::vector<LootTable::LootSlot> &LootTable::getGuaranteedSlots() const {
  return GuaranteedSlots;
}

void LootTable::fillGuaranteedLoot(std::vector<LootReward> &Loot) const {
  for (const auto &Slot : GuaranteedSlots) {
    if (!Slot.LC) {
      continue;
    }
    Slot.LC->fillLoot(Loot);
  }
}

void LootTable::fillLoot(std::vector<LootReward> &Loot) const {
  fillGuaranteedLoot(Loot);
  std::vector<LootSlot> LeftOverSlots = Slots;
  for (unsigned Cnt = 0; Cnt < NumRolls; Cnt++) {
    if (LeftOverSlots.empty()) {
      break;
    }

    auto SlotIdx = rollForSlot(LeftOverSlots);
    const auto &Slot = LeftOverSlots.at(SlotIdx);
    if (Slot.LC) {
      Slot.LC->fillLoot(Loot);
    }

    // Remove the slot so the given entry can not be included again
    // in the loot rewards
    LeftOverSlots.erase(LeftOverSlots.begin() + SlotIdx);
  }
}

} // namespace rogue