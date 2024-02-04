#include "ItemsCommon.h"
#include <gtest/gtest.h>
#include <rogue/ItemPrototype.h>

namespace {

using PId = rogue::ItemProtoId;

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
  rogue::ItemPrototype Proto(PId(1), "Name", "Description",
                             rogue::ItemType::None, 1, {});
  EXPECT_EQ(Proto.ItemId, 1);
  EXPECT_EQ(Proto.Name, "Name");
  EXPECT_EQ(Proto.Description, "Description");
  EXPECT_EQ(Proto.Type, rogue::ItemType::None);
  EXPECT_EQ(Proto.MaxStackSize, 1);
  EXPECT_EQ(Proto.Effects.size(), 0);
  EXPECT_EQ(Proto.getAttributes(), rogue::EffectAttributes{});
  EXPECT_EQ(Proto.getCapabilityFlags(), rogue::CapabilityFlags::None);
  EXPECT_EQ(Proto.hasEffect(rogue::CapabilityFlags::UseOn), false);
}

TEST_F(ItemPrototypeTest, GetCapabilityFlags) {
  rogue::ItemPrototype Proto(
      PId(1), "Name", "Description", rogue::ItemType::Consumable, 1,
      {{{rogue::CapabilityFlags::UseOn}, rogue::test::DummyItems::NullEffect}});
  EXPECT_EQ(Proto.getCapabilityFlags(), rogue::CapabilityFlags::UseOn);
  EXPECT_EQ(Proto.getAttributes(),
            rogue::EffectAttributes{rogue::CapabilityFlags::UseOn});
  EXPECT_FALSE(Proto.hasEffect(rogue::CapabilityFlags::UseOn));
  EXPECT_TRUE(Proto.hasEffect(rogue::CapabilityFlags::UseOn, true));

  rogue::ItemPrototype Proto2(PId(1), "Name", "Description",
                              rogue::ItemType::Ring, 1,
                              {{{rogue::CapabilityFlags::Equipment},
                                rogue::test::DummyItems::NullEffect}});
  EXPECT_EQ(Proto2.getCapabilityFlags(), rogue::CapabilityFlags::Equipment);
  EXPECT_EQ(Proto2.getAttributes(),
            rogue::EffectAttributes{rogue::CapabilityFlags::Equipment});

  rogue::ItemPrototype Proto3(PId(1), "Name", "Description",
                              rogue::ItemType::Crafting, 1, {});
  EXPECT_EQ(Proto3.getCapabilityFlags(), rogue::CapabilityFlags::None);
  EXPECT_EQ(Proto3.getAttributes(),
            rogue::EffectAttributes{rogue::CapabilityFlags::None});
}

TEST_F(ItemPrototypeTest, CanApply) {
  rogue::ItemPrototype Proto(
      PId(1), "Name", "Description", rogue::ItemType::Consumable, 1,
      {{{rogue::CapabilityFlags::UseOn}, rogue::test::DummyItems::NullEffect}});
  EXPECT_FALSE(
      Proto.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::None))
      << "Can't apply with no flags";
  EXPECT_TRUE(
      Proto.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn))
      << "Can apply with matching flag";
  EXPECT_FALSE(
      Proto.canApplyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment))
      << "Can't apply with non-matching flag";
}

TEST_F(ItemPrototypeTest, CanApplyCostHP) {
  rogue::ItemPrototype Proto(PId(1), "Name", "Description",
                             rogue::ItemType::Consumable, 1,
                             {{{rogue::CapabilityFlags::UseOn, 0, 0, 10},
                               rogue::test::DummyItems::NullEffect}});
  auto SrcEt = Reg.create();
  EXPECT_FALSE(
      Proto.canApplyTo(SrcEt, Entity, Reg, rogue::CapabilityFlags::UseOn))
      << "Can't apply with no health component";
  auto &HC = Reg.emplace<rogue::HealthComp>(SrcEt);
  HC.Value = 5;
  EXPECT_FALSE(
      Proto.canApplyTo(SrcEt, Entity, Reg, rogue::CapabilityFlags::UseOn))
      << "Can't apply with not enough HP";
  HC.Value = 15;
  EXPECT_TRUE(
      Proto.canApplyTo(SrcEt, Entity, Reg, rogue::CapabilityFlags::UseOn))
      << "Can apply with enough HP";
}

TEST_F(ItemPrototypeTest, CanApplyCostAP) {
  rogue::ItemPrototype Proto(PId(1), "Name", "Description",
                             rogue::ItemType::Consumable, 1,
                             {{{rogue::CapabilityFlags::UseOn, 10, 0, 0},
                               rogue::test::DummyItems::NullEffect}});
  auto SrcEt = Reg.create();
  EXPECT_FALSE(
      Proto.canApplyTo(SrcEt, Entity, Reg, rogue::CapabilityFlags::UseOn))
      << "Can't apply with no agility component";
  auto &AC = Reg.emplace<rogue::AgilityComp>(SrcEt);
  AC.AP = 5;
  EXPECT_TRUE(
      Proto.getAttributes(rogue::CapabilityFlags::UseOn).checkCosts(SrcEt, Reg))
      << "Can apply with enough AP";
  EXPECT_TRUE(Proto.Effects.at(0).canApplyTo(SrcEt, Entity, Reg,
                                             rogue::CapabilityFlags::UseOn))
      << "Can apply with enough AP";
  EXPECT_TRUE(
      Proto.canApplyTo(SrcEt, Entity, Reg, rogue::CapabilityFlags::UseOn))
      << "Can apply with not enough AP";
}

TEST_F(ItemPrototypeTest, Apply) {
  rogue::ItemPrototype Proto(
      PId(1), "Name", "Description", rogue::ItemType::Consumable, 1,
      {{{rogue::CapabilityFlags::UseOn}, rogue::test::DummyItems::NullEffect}});
  EXPECT_THROW(Proto.applyTo(Entity, Entity, Reg, rogue::CapabilityFlags::None),
               std::runtime_error)
      << "Can't apply with no flags";
  EXPECT_NO_THROW(
      Proto.applyTo(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn))
      << "Can apply with matching flag";
  try {
    Proto.applyTo(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment);
    FAIL() << "Can't apply with non-matching flag";
  } catch (const std::runtime_error &E) {
    EXPECT_EQ(
        E.what(),
        std::string("Can't apply item 'Name' to entity with flags Equipment "
                    "(item flags: Use, item type: Consumable, attrs: "
                    "EffectAttributes{Flags: Use})"));
  }
}

TEST_F(ItemPrototypeTest, ApplyCost) {
  rogue::ItemPrototype Proto(PId(1), "Name", "Description",
                             rogue::ItemType::Consumable, 1,
                             {{{rogue::CapabilityFlags::UseOn, 10, 10, 10},
                               rogue::test::DummyItems::NullEffect}});
  auto SrcEt = Reg.create();
  auto &AC = Reg.emplace<rogue::AgilityComp>(SrcEt);
  AC.AP = 5;
  auto &HC = Reg.emplace<rogue::HealthComp>(SrcEt);
  HC.Value = 15;
  auto &MC = Reg.emplace<rogue::ManaComp>(SrcEt);
  MC.Value = 15;
  EXPECT_NO_THROW(
      Proto.applyTo(SrcEt, Entity, Reg, rogue::CapabilityFlags::UseOn))
      << "Can apply with enough HP/MP/AP";
  EXPECT_EQ(AC.AP, -5) << "AP not reduced by cost";
  EXPECT_EQ(HC.Value, 5) << "HP reduced by cost";
  EXPECT_EQ(MC.Value, 5) << "MP reduced by cost";
}

TEST_F(ItemPrototypeTest, CanRemove) {
  rogue::ItemPrototype Proto(
      PId(1), "Name", "Description", rogue::ItemType::Consumable, 1,
      {{{rogue::CapabilityFlags::UseOn}, rogue::test::DummyItems::NullEffect}});
  EXPECT_FALSE(
      Proto.canRemoveFrom(Entity, Entity, Reg, rogue::CapabilityFlags::None))
      << "Can't remove with no flags";
  EXPECT_TRUE(
      Proto.canRemoveFrom(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn))
      << "Can remove with matching flag";
  EXPECT_FALSE(Proto.canRemoveFrom(Entity, Entity, Reg,
                                   rogue::CapabilityFlags::Equipment))
      << "Can't remove with non-matching flag";
}

TEST_F(ItemPrototypeTest, Remove) {
  rogue::ItemPrototype Proto(
      PId(1), "Name", "Description", rogue::ItemType::Consumable, 1,
      {{{rogue::CapabilityFlags::UseOn}, rogue::test::DummyItems::NullEffect}});
  EXPECT_THROW(
      Proto.removeFrom(Entity, Entity, Reg, rogue::CapabilityFlags::None),
      std::runtime_error)
      << "Can't remove with no flags";
  EXPECT_NO_THROW(
      Proto.removeFrom(Entity, Entity, Reg, rogue::CapabilityFlags::UseOn))
      << "Can remove with matching flag";
  try {
    Proto.removeFrom(Entity, Entity, Reg, rogue::CapabilityFlags::Equipment);
    FAIL() << "Can't remove with non-matching flag";
  } catch (const std::runtime_error &E) {
    EXPECT_EQ(
        E.what(),
        std::string("Can't remove item 'Name' from entity with flags Equipment "
                    "(item flags: Use, item type: Consumable, attrs: "
                    "EffectAttributes{Flags: Use})"));
  }
}

} // namespace