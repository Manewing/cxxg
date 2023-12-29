#include "ItemsCommon.h"
#include <gtest/gtest.h>
#include <rogue/ItemPrototype.h>

namespace {

class ItemPrototypeTest : public ::testing::Test {
public:
  void SetUp() override {
    Reg = entt::registry();
    Entity = Reg.create();
  }

  entt::registry Reg;
  entt::entity Entity;
};

TEST_F(ItemPrototypeTest, Properties) {
  rogue::ItemPrototype Proto(1, "Name", "Description", rogue::ItemType::None, 1,
                             {});
  EXPECT_EQ(Proto.ItemId, 1);
  EXPECT_EQ(Proto.Name, "Name");
  EXPECT_EQ(Proto.Description, "Description");
  EXPECT_EQ(Proto.Type, rogue::ItemType::None);
  EXPECT_EQ(Proto.MaxStackSize, 1);
  EXPECT_EQ(Proto.Effects.size(), 0);
}

TEST_F(ItemPrototypeTest, GetCapabilityFlags) {
  rogue::ItemPrototype Proto(
      1, "Name", "Description", rogue::ItemType::Consumable, 1,
      {{{rogue::CapabilityFlags::UseOn}, rogue::test::DummyItems::NullEffect}});
  EXPECT_EQ(Proto.getCapabilityFlags(), rogue::CapabilityFlags::UseOn);

  rogue::ItemPrototype Proto2(1, "Name", "Description", rogue::ItemType::Ring,
                              1,
                              {{{rogue::CapabilityFlags::Equipment},
                                rogue::test::DummyItems::NullEffect}});
  EXPECT_EQ(Proto2.getCapabilityFlags(), rogue::CapabilityFlags::Equipment);

  rogue::ItemPrototype Proto3(1, "Name", "Description",
                              rogue::ItemType::Crafting, 1, {});
  EXPECT_EQ(Proto3.getCapabilityFlags(), rogue::CapabilityFlags::None);
}

TEST_F(ItemPrototypeTest, CanApply) {
  rogue::ItemPrototype Proto(
      1, "Name", "Description", rogue::ItemType::Consumable, 1,
      {{{rogue::CapabilityFlags::UseOn}, rogue::test::DummyItems::NullEffect}});
  EXPECT_FALSE(Proto.canApplyTo(Entity, Reg, rogue::CapabilityFlags::None))
      << "Can't apply with no flags";
  EXPECT_TRUE(Proto.canApplyTo(Entity, Reg, rogue::CapabilityFlags::UseOn))
      << "Can apply with matching flag";
  EXPECT_FALSE(Proto.canApplyTo(Entity, Reg, rogue::CapabilityFlags::Equipment))
      << "Can't apply with non-matching flag";
}

TEST_F(ItemPrototypeTest, Apply) {
  rogue::ItemPrototype Proto(
      1, "Name", "Description", rogue::ItemType::Consumable, 1,
      {{{rogue::CapabilityFlags::UseOn}, rogue::test::DummyItems::NullEffect}});
  EXPECT_THROW(Proto.applyTo(Entity, Reg, rogue::CapabilityFlags::None),
               std::runtime_error)
      << "Can't apply with no flags";
  EXPECT_NO_THROW(Proto.applyTo(Entity, Reg, rogue::CapabilityFlags::UseOn))
      << "Can apply with matching flag";
  try {
    Proto.applyTo(Entity, Reg, rogue::CapabilityFlags::Equipment);
    FAIL() << "Can't apply with non-matching flag";
  } catch (const std::runtime_error &E) {
    EXPECT_EQ(
        E.what(),
        std::string("Can't apply item 'Name' to entity with flags Equipment "
                    "(item flags: Use, item type: Consumable)"));
  }
}

TEST_F(ItemPrototypeTest, CanRemove) {
  rogue::ItemPrototype Proto(
      1, "Name", "Description", rogue::ItemType::Consumable, 1,
      {{{rogue::CapabilityFlags::UseOn}, rogue::test::DummyItems::NullEffect}});
  EXPECT_FALSE(Proto.canRemoveFrom(Entity, Reg, rogue::CapabilityFlags::None))
      << "Can't remove with no flags";
  EXPECT_TRUE(Proto.canRemoveFrom(Entity, Reg, rogue::CapabilityFlags::UseOn))
      << "Can remove with matching flag";
  EXPECT_FALSE(
      Proto.canRemoveFrom(Entity, Reg, rogue::CapabilityFlags::Equipment))
      << "Can't remove with non-matching flag";
}

TEST_F(ItemPrototypeTest, Remove) {
  rogue::ItemPrototype Proto(
      1, "Name", "Description", rogue::ItemType::Consumable, 1,
      {{{rogue::CapabilityFlags::UseOn}, rogue::test::DummyItems::NullEffect}});
  EXPECT_THROW(Proto.removeFrom(Entity, Reg, rogue::CapabilityFlags::None),
               std::runtime_error)
      << "Can't remove with no flags";
  EXPECT_NO_THROW(Proto.removeFrom(Entity, Reg, rogue::CapabilityFlags::UseOn))
      << "Can remove with matching flag";
  try {
    Proto.removeFrom(Entity, Reg, rogue::CapabilityFlags::Equipment);
    FAIL() << "Can't remove with non-matching flag";
  } catch (const std::runtime_error &E) {
    EXPECT_EQ(
        E.what(),
        std::string("Can't remove item 'Name' from entity with flags Equipment "
                    "(item flags: Use, item type: Consumable)"));
  }
}

} // namespace