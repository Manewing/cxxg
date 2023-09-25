#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <rogue/Components/Helpers.h>
#include <rogue/Components/Stats.h>

namespace {

TEST(ComponentHelpersTest, CopyComponent) {
  entt::registry RegA, RegB;

  auto EntityA = RegA.create();
  RegA.emplace<rogue::StatsComp>(EntityA);

  // Sanity check once on new component
  auto EntityB = RegB.create();
  ASSERT_FALSE(RegB.any_of<rogue::StatsComp>(EntityB));

  rogue::copyComponent<rogue::StatsComp>(EntityA, RegA, EntityB, RegB);
  EXPECT_TRUE(RegB.any_of<rogue::StatsComp>(EntityB));
}

TEST(ComponentHelpersTest, ApplyForComponents) {
  using CL = rogue::ComponentList<rogue::StatsComp, rogue::HealthComp,
                                  rogue::ManaComp>;

  entt::registry Reg;

  auto Entity = Reg.create();

  rogue::applyForComponents<CL>([Entity, &Reg](auto &Arg) {
    Reg.emplace<std::decay_t<decltype(Arg)>>(Entity);
  });

  EXPECT_TRUE(Reg.any_of<rogue::StatsComp>(Entity));
  EXPECT_TRUE(Reg.any_of<rogue::HealthComp>(Entity));
  EXPECT_TRUE(Reg.any_of<rogue::ManaComp>(Entity));
}

TEST(ComponentHelpersTest, AddComponents) {
  using CL = rogue::ComponentList<rogue::StatsComp, rogue::HealthComp,
                                  rogue::ManaComp>;

  entt::registry Reg;

  auto Entity = Reg.create();

  rogue::addComponents<CL>(Entity, Reg);

  EXPECT_TRUE(Reg.any_of<rogue::StatsComp>(Entity));
  EXPECT_TRUE(Reg.any_of<rogue::HealthComp>(Entity));
  EXPECT_TRUE(Reg.any_of<rogue::ManaComp>(Entity));
}

TEST(ComponentHelpersTest, CopyComponents) {
  using CL = rogue::ComponentList<rogue::StatsComp, rogue::HealthComp,
                                  rogue::ManaComp>;

  entt::registry RegA, RegB;

  auto EntityA = RegA.create();
  auto EntityB = RegB.create();

  rogue::addComponents<CL>(EntityA, RegA);
  rogue::copyComponents<CL>(EntityA, RegA, EntityB, RegB);

  EXPECT_TRUE(RegB.any_of<rogue::StatsComp>(EntityB));
  EXPECT_TRUE(RegB.any_of<rogue::HealthComp>(EntityB));
  EXPECT_TRUE(RegB.any_of<rogue::ManaComp>(EntityB));
}

TEST(ComponentsHelpersTest, RemoveComponents) {
  using CL = rogue::ComponentList<rogue::StatsComp, rogue::HealthComp,
                                  rogue::ManaComp>;

  entt::registry Reg;

  auto Entity = Reg.create();

  rogue::addComponents<CL>(Entity, Reg);

  ASSERT_TRUE(Reg.any_of<rogue::StatsComp>(Entity));
  ASSERT_TRUE(Reg.any_of<rogue::HealthComp>(Entity));
  ASSERT_TRUE(Reg.any_of<rogue::ManaComp>(Entity));

  rogue::removeComponents<CL>(Entity, Reg);

  EXPECT_FALSE(Reg.any_of<rogue::StatsComp>(Entity));
  EXPECT_FALSE(Reg.any_of<rogue::HealthComp>(Entity));
  EXPECT_FALSE(Reg.any_of<rogue::ManaComp>(Entity));
}

} // namespace