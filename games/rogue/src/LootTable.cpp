#include <rogue/LootTable.h>

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

LootTable::LootTable(unsigned NumRolls, const std::vector<LootSlot> &Sls)
    : NumRolls(NumRolls) {
  reset(NumRolls, Sls);
}

void LootTable::reset(unsigned NR, const std::vector<LootSlot> &Sls) {
  NumRolls = NR;
  for (const auto &Slot : Sls) {
    if (Slot.Weight == -1) {
      GuaranteedSlots.push_back(Slot);
    } else {
      Slots.push_back(Slot);
    }
  }

  TotalWeight = 0;
  for (const auto &Slot : Slots) {
    TotalWeight += Slot.Weight;
  }
}

const LootTable::LootSlot &LootTable::getSlotForRoll(int Roll) const {
  assert(Roll >= 0 && Roll < TotalWeight);
  for (const auto &Slot : Slots) {
    Roll -= Slot.Weight;
    if (Roll < 0) {
      return Slot;
    }
  }
  throw std::runtime_error("LootTable::getSlotForRoll() failed");
}

const LootTable::LootSlot &LootTable::rollForSlot() const {
  // FIXME this needs to be based on the seed as well
  int Roll = std::rand() % TotalWeight;
  return getSlotForRoll(Roll);
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
  for (unsigned Cnt = 0; Cnt < NumRolls; Cnt++) {
    const auto &Slot = rollForSlot();
    if (!Slot.LC) {
      continue;
    }
    Slot.LC->fillLoot(Loot);
  }
}

} // namespace rogue