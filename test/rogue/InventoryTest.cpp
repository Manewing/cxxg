#include <gtest/gtest.h>
#include <rogue/Inventory.h>
#include <rogue/ItemEffect.h>
#include <rogue/ItemPrototype.h>

namespace {

const rogue::ItemPrototype DummyConsumable(
    1, "consumable", "desc", rogue::ItemType::Consumable, 5,
    {{rogue::CapabilityFlags::UseOn, std::make_shared<rogue::ItemEffect>()}});

TEST(InventoryTest, Empty) {
  rogue::Inventory Inv;
  EXPECT_EQ(Inv.size(), 0);
  EXPECT_TRUE(Inv.empty());
  EXPECT_THROW(Inv.getItem(0), std::out_of_range);
}

TEST(InventoryTest, AddItem) {
  rogue::Inventory Inv;
  rogue::Item It(DummyConsumable, 2);
  Inv.addItem(It);
  ASSERT_EQ(Inv.size(), 1);
  EXPECT_EQ(Inv.getItem(0).StackSize, 2);
  EXPECT_EQ(Inv.getItem(0).getName(), "consumable");
  Inv.addItem(It);
  ASSERT_EQ(Inv.size(), 1);
  EXPECT_EQ(Inv.getItem(0).StackSize, 4);
  Inv.addItem(It);
  ASSERT_EQ(Inv.size(), 2);
  EXPECT_EQ(Inv.getItem(0).StackSize, 5);
  EXPECT_EQ(Inv.getItem(1).StackSize, 1);
  Inv.addItem(It);
  Inv.addItem(It);
  Inv.addItem(It);
  ASSERT_EQ(Inv.size(), 3);
  EXPECT_EQ(Inv.getItem(0).StackSize, 5);
  EXPECT_EQ(Inv.getItem(1).StackSize, 5);
  EXPECT_EQ(Inv.getItem(2).StackSize, 2);
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

TEST(InventoryTest, ApplyItemToUseConsumable) {
  rogue::Item It(DummyConsumable);
  entt::registry Reg;
  entt::entity Entity = Reg.create();
  EXPECT_TRUE(rogue::Inventory::applyItemTo(It, rogue::CapabilityFlags::UseOn,
                                            Entity, Reg));
}

TEST(InventoryTest, ApplyItemToUseConsumableFromInv) {
  rogue::Inventory Inv;
  Inv.addItem(rogue::Item(DummyConsumable, 4));
  entt::registry Reg;
  entt::entity Entity = Reg.create();
  EXPECT_TRUE(Inv.applyItemTo(0, rogue::CapabilityFlags::UseOn, Entity, Reg));

  ASSERT_EQ(Inv.size(), 1);
  EXPECT_EQ(Inv.getItem(0).StackSize, 3);
}

} // namespace