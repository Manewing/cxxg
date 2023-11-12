#include <gtest/gtest.h>
#include <rogue/ItemDatabase.h>

namespace {

TEST(ItemDatabaseTest, Empty) {
  rogue::ItemDatabase Db;
  EXPECT_THROW(Db.getItemId("foo"), std::out_of_range);
  EXPECT_THROW(Db.getRandomItemId(), std::out_of_range);
  EXPECT_THROW(Db.createItem(0), std::out_of_range);
}

TEST(ItemDatabaseTest, AddLootTable) {
  rogue::ItemDatabase Db;
  auto &LootTable = Db.addLootTable("foo");
  EXPECT_TRUE(LootTable.getSlots().empty());
  EXPECT_EQ(Db.getLootTable("foo").get(), &LootTable);

  EXPECT_THROW(Db.addLootTable("foo"), std::out_of_range);
}

} // namespace