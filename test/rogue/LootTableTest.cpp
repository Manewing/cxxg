#include <gtest/gtest.h>
#include <rogue/LootTable.h>

namespace {

using PId = rogue::ItemProtoId;

TEST(LootTableTest, LootItemFillLoot) {
  rogue::LootItem LI(PId(1), 42, 42);
  std::vector<rogue::LootContainer::LootReward> LootRef = {{PId(1), 42}};
  std::vector<rogue::LootContainer::LootReward> Loot;
  LI.fillLoot(Loot);
  EXPECT_EQ(Loot, LootRef);
}

TEST(LootTableTest, EmptyLootTable) {
  rogue::LootTable LTB;
  auto Loot = LTB.generateLoot();
  EXPECT_EQ(Loot.size(), 0);
}

TEST(LootTableTest, SimpleLoot) {
  rogue::LootTable LTB(
      3, {
             {std::make_shared<rogue::LootItem>(PId(1), 1, 1), 5},
             {std::make_shared<rogue::LootItem>(PId(2), 1, 1), 10},
             {std::make_shared<rogue::LootItem>(PId(3), 1, 1), 20},
         });

  // Sum of weight: 35
  // Rnd Weight: 0-35
  EXPECT_EQ(LTB.getSlotForRoll(0, LTB.getSlots()), 0);
  EXPECT_EQ(LTB.getSlotForRoll(5, LTB.getSlots()), 1);
  EXPECT_EQ(LTB.getSlotForRoll(6, LTB.getSlots()), 1);
  EXPECT_EQ(LTB.getSlotForRoll(12, LTB.getSlots()), 1);
  EXPECT_EQ(LTB.getSlotForRoll(15, LTB.getSlots()), 2);
  EXPECT_EQ(LTB.getSlotForRoll(30, LTB.getSlots()), 2);

  std::srand(0);
  auto Loot = LTB.generateLoot();
  EXPECT_EQ(Loot.size(), 3);
}

TEST(LootTableTest, GuaranteedRewards) {
  rogue::LootTable LTB(
      1, {
             {std::make_shared<rogue::LootItem>(PId(1), 1, 1), 5},
             {std::make_shared<rogue::LootItem>(PId(2), 1, 1), 10},
             {std::make_shared<rogue::LootItem>(PId(3), 1, 1), 20},
             {std::make_shared<rogue::LootItem>(PId(4), 1, 1), -1},
         });
  std::vector<rogue::LootContainer::LootReward> LootRef = {{PId(4), 1}};
  std::vector<rogue::LootContainer::LootReward> Loot;
  LTB.fillGuaranteedLoot(Loot);
  EXPECT_EQ(Loot, LootRef);

  Loot = LTB.generateLoot();
  EXPECT_EQ(Loot.size(), 2);

  // Check reset
  LTB.reset(1, {});
  Loot = LTB.generateLoot();
  EXPECT_EQ(Loot.size(), 0);
}

TEST(LootTableTest, Empty) {
  rogue::LootTable LTB;
  auto Loot = LTB.generateLoot();
  EXPECT_TRUE(Loot.empty());
}

TEST(LootTableTest, NullLoot) {
  rogue::LootTable LTB(1, {
                              {nullptr, 20},
                              {nullptr, -1},
                          });
  auto Loot = LTB.generateLoot();
  EXPECT_TRUE(Loot.empty());
}

TEST(LootTableTest, NestedLootTables) {
  auto CoinsLTB = std::make_shared<rogue::LootTable>(
      2, std::vector<rogue::LootTable::LootSlot>{
             {std::make_shared<rogue::LootItem>(PId(4), 80, 80), 10},
             {std::make_shared<rogue::LootItem>(PId(4), 40, 40), 50},
             {std::make_shared<rogue::LootItem>(PId(4), 20, 20), 100},
         });
  rogue::LootTable LTB(1, {
                              {std::make_shared<rogue::LootItem>(PId(1), 1, 1), 5},
                              {std::make_shared<rogue::LootItem>(PId(2), 1, 1), 10},
                              {std::make_shared<rogue::LootItem>(PId(3), 1, 1), 20},
                              {CoinsLTB, -1},
                          });

  std::srand(0);
  auto Loot = LTB.generateLoot();
  EXPECT_EQ(Loot.size(), 3);
  EXPECT_EQ(Loot.at(0).ItId, 4);
  EXPECT_EQ(Loot.at(1).ItId, 4);
}

} // namespace