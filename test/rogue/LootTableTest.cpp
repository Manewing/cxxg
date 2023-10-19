#include <gtest/gtest.h>
#include <rogue/LootTable.h>

namespace {

rogue::LootContainer::ItemId getLootItemId(const rogue::LootContainer &LC) {
  if (const auto *LI = dynamic_cast<const rogue::LootItem *>(&LC)) {
    return LI->getItemId();
  }
  return -1;
}

TEST(LootTableTest, LootItemFillLoot) {
  rogue::LootItem LI(1, 42, 42);
  std::vector<rogue::LootContainer::LootReward> LootRef = {{1, 42}};
  std::vector<rogue::LootContainer::LootReward> Loot;
  LI.fillLoot(Loot);
  EXPECT_EQ(Loot, LootRef);
}

TEST(LootTableTest, SimpleLoot) {
  rogue::LootTable LTB(3, {
                              {std::make_shared<rogue::LootItem>(1, 1, 1), 5},
                              {std::make_shared<rogue::LootItem>(2, 1, 1), 10},
                              {std::make_shared<rogue::LootItem>(3, 1, 1), 20},
                          });

  // Sum of weight: 35
  // Rnd Weight: 0-35
  EXPECT_EQ(getLootItemId(*LTB.getSlotForRoll(0).LC), 1);
  EXPECT_EQ(getLootItemId(*LTB.getSlotForRoll(5).LC), 2);
  EXPECT_EQ(getLootItemId(*LTB.getSlotForRoll(6).LC), 2);
  EXPECT_EQ(getLootItemId(*LTB.getSlotForRoll(12).LC), 2);
  EXPECT_EQ(getLootItemId(*LTB.getSlotForRoll(15).LC), 3);
  EXPECT_EQ(getLootItemId(*LTB.getSlotForRoll(30).LC), 3);

  std::srand(0);
  auto Loot = LTB.generateLoot();
  EXPECT_EQ(Loot.size(), 3);
}

TEST(LootTableTest, GuaranteedRewards) {
  rogue::LootTable LTB(1, {
                              {std::make_shared<rogue::LootItem>(1, 1, 1), 5},
                              {std::make_shared<rogue::LootItem>(2, 1, 1), 10},
                              {std::make_shared<rogue::LootItem>(3, 1, 1), 20},
                              {std::make_shared<rogue::LootItem>(4, 1, 1), -1},
                          });
  std::vector<rogue::LootContainer::LootReward> LootRef = {{4, 1}};
  std::vector<rogue::LootContainer::LootReward> Loot;
  LTB.fillGuaranteedLoot(Loot);
  EXPECT_EQ(Loot, LootRef);

  Loot = LTB.generateLoot();
  EXPECT_EQ(Loot.size(), 2);
}

TEST(LootTableTest, NullLoot) {
  rogue::LootTable LTB(1, {
                              {nullptr, 20},
                              {nullptr, -1},
                          });
  std::vector<rogue::LootContainer::LootReward> LootRef ;
  auto Loot = LTB.generateLoot();
  EXPECT_TRUE(Loot.empty());
}

TEST(LootTableTest, NestedLootTables) {
  auto CoinsLTB = std::make_shared<rogue::LootTable>(
      2, std::vector<rogue::LootTable::LootSlot>{
             {std::make_shared<rogue::LootItem>(4, 80, 80), 10},
             {std::make_shared<rogue::LootItem>(4, 40, 40), 50},
             {std::make_shared<rogue::LootItem>(4, 20, 20), 100},
         });
  rogue::LootTable LTB(1, {
                              {std::make_shared<rogue::LootItem>(1, 1, 1), 5},
                              {std::make_shared<rogue::LootItem>(2, 1, 1), 10},
                              {std::make_shared<rogue::LootItem>(3, 1, 1), 20},
                              {CoinsLTB, -1},
                          });

  std::srand(0);
  auto Loot = LTB.generateLoot();
  EXPECT_EQ(Loot.size(), 3);
  EXPECT_EQ(Loot.at(0).ItId, 4);
  EXPECT_EQ(Loot.at(1).ItId, 4);
}

} // namespace