#ifndef ROGUE_LOOT_TABLE_H
#define ROGUE_LOOT_TABLE_H

#include <memory>
#include <vector>

namespace rogue {

class LootContainer {
public:
  using ItemId = int;

  struct LootReward {
    ItemId ItId = -1;
    unsigned Count = 0;
  };

public:
  virtual ~LootContainer() = default;
  virtual void fillLoot(std::vector<LootReward> &Loot) const = 0;
  std::vector<LootReward> generateLoot() const;
};

inline bool operator==(const LootContainer::LootReward &Lhs,
                       const LootContainer::LootReward &Rhs) noexcept {
  return Lhs.ItId == Rhs.ItId && Lhs.Count == Rhs.Count;
}

class LootItem : public LootContainer {
public:
  LootItem() = delete;
  LootItem(ItemId ItId, unsigned MinCount, unsigned MaxCount);

  ItemId getItemId() const;

  void fillLoot(std::vector<LootReward> &Loot) const final;

private:
  ItemId ItId;
  unsigned MinCount = 0;
  unsigned MaxCount = 0;
};

class LootTable : public LootContainer {
public:
  struct LootSlot {
    /// Loot container for the slot, nullptr to indicate no item
    std::shared_ptr<LootContainer> LC = nullptr;

    /// Weight for the slot
    int Weight = 0;
  };

public:
  static std::size_t getSlotForRoll(int Roll,
                                    const std::vector<LootSlot> &Slots);
  static std::size_t rollForSlot(const std::vector<LootSlot> &Slots);

public:
  LootTable();
  explicit LootTable(unsigned NumRolls, const std::vector<LootSlot> &Slots);
  void reset(unsigned NumRolls, const std::vector<LootSlot> &Slots);

  const std::vector<LootSlot> &getSlots() const;
  const std::vector<LootSlot> &getGuaranteedSlots() const;

  void fillGuaranteedLoot(std::vector<LootReward> &Loot) const;
  void fillLoot(std::vector<LootReward> &Loot) const final;

private:
  unsigned NumRolls = 1;
  std::vector<LootSlot> Slots;
  std::vector<LootSlot> GuaranteedSlots;
};

} // namespace rogue

#endif // #ifndef ROGUE_LOOT_TABLE_H