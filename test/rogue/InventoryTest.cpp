#include <gtest/gtest.h>
#include <rogue/Inventory.h>
#include <rogue/ItemEffect.h>
#include <rogue/ItemPrototype.h>

namespace {

using PId = rogue::ItemProtoId;

const rogue::ItemPrototype DummyConsumable(
    PId(1), "consumable", "desc", rogue::ItemType::Consumable, 5,
    {{{rogue::CapabilityFlags::UseOn}, std::make_shared<rogue::NullEffect>()}});

TEST(InventoryTest, Empty) {
  rogue::Inventory Inv;
  EXPECT_EQ(Inv.size(), 0);
  EXPECT_TRUE(Inv.empty());
  EXPECT_THROW(Inv.getItem(0), std::out_of_range);
  EXPECT_THROW(Inv.takeItem(0), std::out_of_range);
  EXPECT_EQ(Inv.getItemIndexForId(0), std::nullopt);
}

TEST(InventoryTest, AddItem) {
  rogue::Inventory Inv;
  rogue::Item It(DummyConsumable, 2);
  Inv.addItem(It);
  ASSERT_EQ(Inv.size(), 1) << Inv;
  EXPECT_EQ(Inv.getItem(0).StackSize, 2);
  EXPECT_EQ(Inv.getItem(0).getName(), "consumable");
  EXPECT_EQ(Inv.getItemIndexForId(1), 0);
  Inv.addItem(It);
  ASSERT_EQ(Inv.size(), 1) << Inv;
  EXPECT_EQ(Inv.getItem(0).StackSize, 4);
  EXPECT_EQ(Inv.getItemIndexForId(1), 0);
  Inv.addItem(It);
  ASSERT_EQ(Inv.size(), 2) << Inv;
  EXPECT_EQ(Inv.getItem(0).StackSize, 5);
  EXPECT_EQ(Inv.getItem(1).StackSize, 1);
  EXPECT_EQ(Inv.getItemIndexForId(1), 0);
  Inv.addItem(It);
  Inv.addItem(It);
  Inv.addItem(It);
  ASSERT_EQ(Inv.size(), 3) << Inv;
  EXPECT_EQ(Inv.getItem(0).StackSize, 5);
  EXPECT_EQ(Inv.getItem(1).StackSize, 5);
  EXPECT_EQ(Inv.getItem(2).StackSize, 2);
  EXPECT_EQ(Inv.getItemIndexForId(1), 0);
}

TEST(InventoryTest, AddItemInvMaxStack) {
  rogue::Inventory Inv(/*MaxStackSize=*/1);
  rogue::Item It(DummyConsumable, 2);
  Inv.addItem(It);
  ASSERT_EQ(Inv.size(), 2) << Inv;
  EXPECT_EQ(Inv.getItem(0).StackSize, 1);
  EXPECT_EQ(Inv.getItem(0).getName(), "consumable");
  EXPECT_EQ(Inv.getItemIndexForId(1), 0);
  Inv.addItem(It);
  ASSERT_EQ(Inv.size(), 4) << Inv;
  EXPECT_EQ(Inv.getItem(0).StackSize, 1);
  EXPECT_EQ(Inv.getItem(3).StackSize, 1);
  EXPECT_EQ(Inv.getItemIndexForId(1), 0);
}

TEST(InventoryTest, TakeItem) {
  rogue::Inventory Inv;
  rogue::Item It(DummyConsumable, 5);
  Inv.addItem(It);
  ASSERT_EQ(Inv.size(), 1);

  EXPECT_EQ(Inv.takeItem(0).StackSize, 5);
  ASSERT_EQ(Inv.size(), 0);

  Inv.addItem(It);
  Inv.addItem(It);
  ASSERT_EQ(Inv.size(), 2);

  EXPECT_EQ(Inv.takeItem(0).StackSize, 5);
  EXPECT_EQ(Inv.takeItem(0).StackSize, 5);
  ASSERT_EQ(Inv.size(), 0);

  Inv.addItem(It);
  Inv.addItem(It);
  ASSERT_EQ(Inv.size(), 2);

  EXPECT_EQ(Inv.takeItem(0, 2).StackSize, 2)
      << "Taking less than stack size should return the correct amount";
  ASSERT_EQ(Inv.size(), 2);
  EXPECT_EQ(Inv.takeItem(0, 5).StackSize, 3)
      << "Taking more than stack size should return the whole stack";
  EXPECT_EQ(Inv.size(), 1);
}

TEST(InventoryTest, HasItem) {
  rogue::Inventory Inv;
  rogue::Item It(DummyConsumable, 5);
  Inv.addItem(It);
  ASSERT_EQ(Inv.size(), 1);

  EXPECT_TRUE(Inv.hasItem(1, 5));
  EXPECT_TRUE(Inv.hasItem(1, 3));
  EXPECT_TRUE(Inv.hasItem(1, 1));
  EXPECT_FALSE(Inv.hasItem(1, 6));
  EXPECT_FALSE(Inv.hasItem(1, 0));
  EXPECT_FALSE(Inv.hasItem(2, 5));

  Inv.addItem(It);
  ASSERT_EQ(Inv.size(), 2);

  EXPECT_TRUE(Inv.hasItem(1, 10));
}

TEST(InventoryTest, ApplyItemToUseConsumable) {
  rogue::Item It(DummyConsumable);
  entt::registry Reg;
  entt::entity Entity = Reg.create();
  EXPECT_TRUE(rogue::Inventory::applyItemTo(It, rogue::CapabilityFlags::UseOn,
                                            Entity, Entity, Reg));
}

TEST(InventoryTest, ApplyItemToUseConsumableFromInv) {
  rogue::Inventory Inv;
  Inv.addItem(rogue::Item(DummyConsumable, 4));
  entt::registry Reg;
  entt::entity Entity = Reg.create();
  EXPECT_TRUE(
      Inv.applyItemTo(0, rogue::CapabilityFlags::UseOn, Entity, Entity, Reg));

  ASSERT_EQ(Inv.size(), 1);
  EXPECT_EQ(Inv.getItem(0).StackSize, 3);
}

} // namespace