#include "ItemsCommon.h"
#include <gtest/gtest.h>
#include <rogue/Item.h>

namespace {

auto DummyEffComp0 =
    rogue::test::DummyComponentEffect<rogue::test::DummyComp<0>>::get();
auto DummyEffComp1 =
    rogue::test::DummyComponentEffect<rogue::test::DummyComp<1>>::get();

auto DummyEffCompReq0 =
    rogue::test::DummyComponentEffect<rogue::test::DummyComp<0>,
                                      rogue::test::DummyRequiredComp<0>>::get();
auto DummyEffCompReq1 =
    rogue::test::DummyComponentEffect<rogue::test::DummyComp<1>,
                                      rogue::test::DummyRequiredComp<1>>::get();

class ItemTest : public ::testing::Test {
public:
  void SetUp() override {
    Reg = entt::registry();
    Entity = Reg.create();
  }

  entt::registry Reg;
  entt::entity Entity;
};

TEST_F(ItemTest, Properties) {
  rogue::ItemPrototype Proto(1, "Test Item", "Test Description",
                             rogue::ItemType::Ring, 1, {});
  rogue::Item Item(Proto);

  EXPECT_EQ(Item.getId(), 1);
  EXPECT_EQ(Item.getName(), "Test Item");
  EXPECT_EQ(Item.getDescription(), "Test Description");
  EXPECT_EQ(Item.getType(), rogue::ItemType::Ring);
  EXPECT_EQ(Item.getMaxStackSize(), 1);
  EXPECT_EQ(Item.getAllEffects().size(), 0);
  EXPECT_EQ(Item.getCapabilityFlags(), rogue::CapabilityFlags::None);
}

TEST_F(ItemTest, Specialization) {
  rogue::ItemPrototype Proto(
      1, "Test Item", "Test Description", rogue::ItemType::Ring, 1,
      {{{rogue::CapabilityFlags::UseOn}, rogue::test::DummyItems::NullEffect}});
  rogue::ItemPrototype SpecProto(2, "Test Item", "Test Description",
                                 rogue::ItemType::Ring, 1,
                                 {{{rogue::CapabilityFlags::Equipment},
                                   rogue::test::DummyItems::NullEffect}});
  auto Specialization = std::make_shared<rogue::ItemPrototype>(SpecProto);
  rogue::Item Item(Proto, 1, Specialization);

  EXPECT_EQ(Item.getId(), 1);
  EXPECT_EQ(Item.getName(), "Test Item");
  EXPECT_EQ(Item.getDescription(), "Test Description");
  EXPECT_EQ(Item.getType(), rogue::ItemType::Ring);
  EXPECT_EQ(Item.getMaxStackSize(), 1);
  EXPECT_EQ(Item.getAllEffects().size(), 2);
  EXPECT_EQ(Item.getCapabilityFlags(),
            rogue::CapabilityFlags::Equipment | rogue::CapabilityFlags::UseOn);
}

TEST_F(ItemTest, CanApplyTo) {
  rogue::ItemPrototype Proto(
      1, "Test Item", "Test Description",
      rogue::ItemType::Ring | rogue::ItemType::Consumable, 1,
      {{{rogue::CapabilityFlags::UseOn}, DummyEffCompReq0}});
  rogue::ItemPrototype SpecProto(
      2, "Test Item", "Test Description", rogue::ItemType::Ring, 1,
      {{{rogue::CapabilityFlags::Equipment}, DummyEffCompReq1}});
  auto Specialization = std::make_shared<rogue::ItemPrototype>(SpecProto);

  rogue::Item Item(Proto);
  EXPECT_FALSE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  EXPECT_FALSE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment));

  Reg.emplace<rogue::test::DummyRequiredComp<0>>(Entity);
  EXPECT_TRUE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  EXPECT_FALSE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment));

  Item = rogue::Item(Proto, 1, Specialization);
  EXPECT_TRUE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  EXPECT_FALSE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment));

  Reg.emplace<rogue::test::DummyRequiredComp<1>>(Entity);
  EXPECT_TRUE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  EXPECT_TRUE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment));

  Item = rogue::Item(Proto, 1, Specialization, /*SpecOverrides=*/true);
  EXPECT_FALSE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  EXPECT_TRUE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment));

  Reg.remove<rogue::test::DummyRequiredComp<1>>(Entity);
  EXPECT_FALSE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  EXPECT_FALSE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment));
}

TEST_F(ItemTest, CanApplyToWithCost) {
  rogue::ItemPrototype Proto(
      1, "Test Item", "Test Description",
      rogue::ItemType::Ring | rogue::ItemType::Consumable, 1,
      {{{rogue::CapabilityFlags::UseOn, 10, 0, 0},
        rogue::test::DummyItems::NullEffect},
       {{rogue::CapabilityFlags::Equipment, 0, 0, 10},
        rogue::test::DummyItems::NullEffect}});
  rogue::ItemPrototype SpecProto(
      2, "Test Item", "Test Description", rogue::ItemType::Ring, 1,
      {{{rogue::CapabilityFlags::Equipment, 0, 0, 10},
        rogue::test::DummyItems::NullEffect}});
  auto Specialization = std::make_shared<rogue::ItemPrototype>(SpecProto);

  auto Item = rogue::Item(Proto, 1, Specialization);
  EXPECT_FALSE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  EXPECT_FALSE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment));

  Reg.emplace<rogue::AgilityComp>(Entity);
  EXPECT_TRUE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  EXPECT_FALSE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment));

  auto &HC = Reg.emplace<rogue::HealthComp>(Entity);
  HC.Value = 15;
  EXPECT_TRUE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  EXPECT_FALSE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment));

  HC.Value = 20;
  EXPECT_TRUE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  EXPECT_TRUE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment));
}

TEST_F(ItemTest, ApplyToWithSpecialization) {
  rogue::ItemPrototype Proto(
      1, "Test Item", "Test Description", rogue::ItemType::Ring, 1,
      {{{rogue::CapabilityFlags::Equipment}, DummyEffComp0}});
  rogue::ItemPrototype SpecProto(
      2, "Test Item", "Test Description", rogue::ItemType::Ring, 1,
      {{{rogue::CapabilityFlags::Equipment}, DummyEffComp1}});
  auto Specialization = std::make_shared<rogue::ItemPrototype>(SpecProto);

  auto Item = rogue::Item(Proto, 1, Specialization);
  Item.applyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment);

  ASSERT_TRUE(Reg.any_of<rogue::test::DummyComp<0>>(Entity));
  EXPECT_EQ(Reg.get<rogue::test::DummyComp<0>>(Entity).Value, 1);

  ASSERT_TRUE(Reg.any_of<rogue::test::DummyComp<1>>(Entity));
  EXPECT_EQ(Reg.get<rogue::test::DummyComp<1>>(Entity).Value, 1);
}

TEST_F(ItemTest, ApplyToWithSpecializationOverride) {
  rogue::ItemPrototype Proto(
      1, "Test Item", "Test Description", rogue::ItemType::Ring, 1,
      {{{rogue::CapabilityFlags::Equipment}, DummyEffComp0}});
  rogue::ItemPrototype SpecProto(
      2, "Test Item", "Test Description", rogue::ItemType::Ring, 1,
      {{{rogue::CapabilityFlags::Equipment}, DummyEffComp1}});
  auto Specialization = std::make_shared<rogue::ItemPrototype>(SpecProto);

  auto Item = rogue::Item(Proto, 1, Specialization, /*SpecOverrides=*/true);
  Item.applyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment);

  EXPECT_FALSE(Reg.any_of<rogue::test::DummyComp<0>>(Entity));

  ASSERT_TRUE(Reg.any_of<rogue::test::DummyComp<1>>(Entity));
  EXPECT_EQ(Reg.get<rogue::test::DummyComp<1>>(Entity).Value, 1);
}

TEST_F(ItemTest, ApplyToWithSpecMutuallyExclusiveFlags) {
  rogue::ItemPrototype Proto(
      1, "Test Item", "Test Description", rogue::ItemType::Ring, 1,
      {{{rogue::CapabilityFlags::Equipment}, DummyEffComp0}});
  rogue::ItemPrototype SpecProto(
      2, "Test Item", "Test Description", rogue::ItemType::Consumable, 1,
      {{{rogue::CapabilityFlags::UseOn}, DummyEffComp1}});
  auto Specialization = std::make_shared<rogue::ItemPrototype>(SpecProto);

  auto Item = rogue::Item(Proto, 1, Specialization);

  EXPECT_TRUE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment));
  EXPECT_TRUE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));

  ASSERT_FALSE(Reg.any_of<rogue::test::DummyComp<0>>(Entity));
  Item.applyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment);
  ASSERT_TRUE(Reg.any_of<rogue::test::DummyComp<0>>(Entity));
  EXPECT_EQ(Reg.get<rogue::test::DummyComp<0>>(Entity).Value, 1);

  ASSERT_FALSE(Reg.any_of<rogue::test::DummyComp<1>>(Entity));
  Item.applyTo(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn);
  ASSERT_TRUE(Reg.any_of<rogue::test::DummyComp<1>>(Entity));
  EXPECT_EQ(Reg.get<rogue::test::DummyComp<1>>(Entity).Value, 1);
}

TEST_F(ItemTest, CanRemoveFrom) {
  rogue::ItemPrototype Proto(
      1, "Test Item", "Test Description",
      rogue::ItemType::Ring | rogue::ItemType::Consumable, 1,
      {{{rogue::CapabilityFlags::UseOn}, DummyEffCompReq0}});
  rogue::ItemPrototype SpecProto(
      2, "Test Item", "Test Description", rogue::ItemType::Ring, 1,
      {{{rogue::CapabilityFlags::Equipment}, DummyEffCompReq1}});
  auto Specialization = std::make_shared<rogue::ItemPrototype>(SpecProto);

  rogue::Item Item(Proto);
  EXPECT_FALSE(
      Item.canRemoveFrom(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  EXPECT_FALSE(Item.canRemoveFrom(Entity, Entity, Reg,
                                  rogue::CapabilityFlags::Equipment));

  Reg.emplace<rogue::test::DummyComp<0>>(Entity);
  Reg.emplace<rogue::test::DummyRequiredComp<0>>(Entity);
  EXPECT_TRUE(
      Item.canRemoveFrom(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  EXPECT_FALSE(Item.canRemoveFrom(Entity, Entity, Reg,
                                  rogue::CapabilityFlags::Equipment));

  Item = rogue::Item(Proto, 1, Specialization);
  EXPECT_TRUE(
      Item.canRemoveFrom(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  EXPECT_FALSE(Item.canRemoveFrom(Entity, Entity, Reg,
                                  rogue::CapabilityFlags::Equipment));

  Reg.emplace<rogue::test::DummyComp<1>>(Entity);
  Reg.emplace<rogue::test::DummyRequiredComp<1>>(Entity);
  EXPECT_TRUE(
      Item.canRemoveFrom(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  EXPECT_TRUE(Item.canRemoveFrom(Entity, Entity, Reg,
                                 rogue::CapabilityFlags::Equipment));

  Item = rogue::Item(Proto, 1, Specialization, /*SpecOverrides=*/true);
  EXPECT_FALSE(
      Item.canRemoveFrom(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  EXPECT_TRUE(Item.canRemoveFrom(Entity, Entity, Reg,
                                 rogue::CapabilityFlags::Equipment));

  Reg.remove<rogue::test::DummyComp<1>>(Entity);
  Reg.remove<rogue::test::DummyRequiredComp<1>>(Entity);
  EXPECT_FALSE(
      Item.canRemoveFrom(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  EXPECT_FALSE(Item.canRemoveFrom(Entity, Entity, Reg,
                                  rogue::CapabilityFlags::Equipment));
}

TEST_F(ItemTest, RemoveFromWithSpecialization) {
  rogue::ItemPrototype Proto(
      1, "Test Item", "Test Description", rogue::ItemType::Ring, 1,
      {{{rogue::CapabilityFlags::Equipment}, DummyEffComp0}});
  rogue::ItemPrototype SpecProto(
      2, "Test Item", "Test Description", rogue::ItemType::Ring, 1,
      {{{rogue::CapabilityFlags::Equipment}, DummyEffComp1}});
  auto Specialization = std::make_shared<rogue::ItemPrototype>(SpecProto);

  auto Item = rogue::Item(Proto, 1, Specialization);
  EXPECT_TRUE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment));
  Item.applyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment);
  Item.removeFrom(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment);

  EXPECT_FALSE(Reg.any_of<rogue::test::DummyComp<0>>(Entity));
  EXPECT_FALSE(Reg.any_of<rogue::test::DummyComp<1>>(Entity));
}

TEST_F(ItemTest, RemoveFromWithSpecializationOverride) {
  rogue::ItemPrototype Proto(
      1, "Test Item", "Test Description", rogue::ItemType::Ring, 1,
      {{{rogue::CapabilityFlags::Equipment}, DummyEffComp0}});
  rogue::ItemPrototype SpecProto(
      2, "Test Item", "Test Description", rogue::ItemType::Ring, 1,
      {{{rogue::CapabilityFlags::Equipment}, DummyEffComp1}});
  auto Specialization = std::make_shared<rogue::ItemPrototype>(SpecProto);

  auto Item = rogue::Item(Proto, 1, Specialization, /*SpecOverrides=*/true);
  EXPECT_TRUE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment));
  Item.applyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment);

  EXPECT_TRUE(Item.canRemoveFrom(Entity, Entity, Reg,
                                 rogue::CapabilityFlags::Equipment));
  Item.removeFrom(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment);

  EXPECT_FALSE(Reg.any_of<rogue::test::DummyComp<0>>(Entity));
  EXPECT_FALSE(Reg.any_of<rogue::test::DummyComp<1>>(Entity));
}

TEST_F(ItemTest, RemoveFromWithSpecMutuallyExclusive) {
  rogue::ItemPrototype Proto(
      1, "Test Item", "Test Description", rogue::ItemType::Ring, 1,
      {{{rogue::CapabilityFlags::Equipment}, DummyEffComp0}});
  rogue::ItemPrototype SpecProto(
      2, "Test Item", "Test Description", rogue::ItemType::Consumable, 1,
      {{{rogue::CapabilityFlags::UseOn}, DummyEffComp1}});
  auto Specialization = std::make_shared<rogue::ItemPrototype>(SpecProto);

  auto Item = rogue::Item(Proto, 1, Specialization);
  EXPECT_TRUE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment));
  Item.applyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment);
  EXPECT_TRUE(
      Item.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  Item.applyTo(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn);

  EXPECT_TRUE(Reg.any_of<rogue::test::DummyComp<0>>(Entity));
  EXPECT_TRUE(Reg.any_of<rogue::test::DummyComp<1>>(Entity));

  EXPECT_TRUE(Item.canRemoveFrom(Entity, Entity, Reg,
                                 rogue::CapabilityFlags::Equipment));
  Item.removeFrom(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment);

  EXPECT_FALSE(Reg.any_of<rogue::test::DummyComp<0>>(Entity));
  EXPECT_TRUE(
      Item.canRemoveFrom(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn));
  Item.removeFrom(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn);
  EXPECT_FALSE(Reg.any_of<rogue::test::DummyComp<1>>(Entity));
}

} // namespace