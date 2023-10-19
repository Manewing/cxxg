#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Stats.h>
#include <rogue/Systems/StatsSystem.h>

namespace {

TEST(StatsSystemTest, StatsSystemUpdateStatsBuff) {
  const rogue::StatPoints StatPoints = {1, 2, 3, 4};

  entt::registry Reg;
  rogue::StatsSystem StatsSystem(Reg);

  auto Entity = Reg.create();

  auto &Stats = Reg.emplace<rogue::StatsComp>(Entity, /*Base=*/StatPoints,
                                              /*Bonus=*/StatPoints);
  StatsSystem.update(
      rogue::System::UpdateType::NoTick); // Should not affect ref
  EXPECT_EQ(Stats.Base, StatPoints);
  EXPECT_EQ(Stats.Bonus, rogue::StatPoints());

  auto &StatsBuff = Reg.emplace<rogue::StatsBuffComp>(Entity);
  StatsBuff.Bonus = StatPoints;
  StatsSystem.update(
      rogue::System::UpdateType::NoTick); // Should not affect ref
  EXPECT_EQ(Reg.get<rogue::StatsComp>(Entity).Base, StatPoints);
  EXPECT_EQ(Reg.get<rogue::StatsComp>(Entity).Bonus, StatPoints);
}

TEST(StatsSystemTest, StatsSystemRegenEffects) {
  const rogue::StatPoints StatPoints = {1, 2, 3, 4};

  entt::registry Reg;
  rogue::StatsSystem StatsSystem(Reg);

  auto Entity = Reg.create();
  Reg.emplace<rogue::StatsComp>(Entity, /*Base=*/StatPoints,
                                /*Bonus=*/rogue::StatPoints());

  auto &Health = Reg.emplace<rogue::HealthComp>(Entity);
  Health.MaxValue = 100;
  Health.Value = 100;

  auto &Mana = Reg.emplace<rogue::ManaComp>(Entity);
  Mana.MaxValue = 100;
  Mana.Value = 100;

  auto &Agility = Reg.emplace<rogue::AgilityComp>(Entity);
  Agility.Agility = 100;

  StatsSystem.update(rogue::System::UpdateType::NoTick);

  EXPECT_EQ(Health.MaxValue, 9.1f * StatPoints.Vit);
  EXPECT_EQ(Health.Value, Health.MaxValue);
  EXPECT_EQ(Mana.MaxValue, 9.1f * StatPoints.Int);
  EXPECT_EQ(Mana.Value, Mana.MaxValue);
  EXPECT_EQ(int(Agility.Agility), 18);
}

TEST(TimedStatsSystem, TimedStatSystemStatsTimedBuff) {
  const rogue::StatPoints StatPoints = {1, 2, 3, 4};

  entt::registry Reg;
  rogue::StatsSystem StatsSystem(Reg);

  auto Entity = Reg.create();

  auto &Stats = Reg.emplace<rogue::StatsComp>(Entity);
  Stats.Base = StatPoints;
  Stats.Bonus = rogue::StatPoints();

  auto &STBC = Reg.emplace<rogue::StatsTimedBuffComp>(Entity);
  STBC.TicksLeft = 2;
  STBC.Bonus = StatPoints;

  StatsSystem.update(rogue::System::UpdateType::Tick);
  EXPECT_EQ(Stats.Base, StatPoints);
  EXPECT_EQ(Stats.Bonus, StatPoints);
  ASSERT_TRUE(Reg.any_of<rogue::StatsTimedBuffComp>(Entity));
  EXPECT_EQ(STBC.TicksLeft, 1);

  StatsSystem.update(rogue::System::UpdateType::Tick);
  EXPECT_EQ(Stats.Base, StatPoints);
  EXPECT_EQ(Stats.Bonus, StatPoints);
  ASSERT_TRUE(Reg.any_of<rogue::StatsTimedBuffComp>(Entity));
  EXPECT_EQ(STBC.TicksLeft, 0);

  StatsSystem.update(rogue::System::UpdateType::Tick);
  EXPECT_EQ(Stats.Base, StatPoints);
  EXPECT_EQ(Stats.Bonus, rogue::StatPoints());
  EXPECT_FALSE(Reg.any_of<rogue::StatsTimedBuffComp>(Entity));
}

TEST(TimedStatsSystem, TimedStatSystemStatsBuffPerHit) {
  const rogue::StatPoints StatPoints = {1, 2, 3, 4};

  entt::registry Reg;
  rogue::StatsSystem StatsSystem(Reg);

  auto Entity = Reg.create();

  auto &Stats = Reg.emplace<rogue::StatsComp>(Entity);
  Stats.Base = StatPoints;
  Stats.Bonus = rogue::StatPoints();

  auto &SBPHC = Reg.emplace<rogue::StatsBuffPerHitComp>(Entity);
  SBPHC.Stacks = 0;
  SBPHC.MaxStacks = 3;
  SBPHC.MaxTicks = 2;
  SBPHC.TicksLeft = 0;
  SBPHC.SBC.Bonus = StatPoints;

  StatsSystem.update(rogue::System::UpdateType::Tick);
  EXPECT_EQ(Stats.Base, StatPoints);
  EXPECT_EQ(Stats.Bonus, rogue::StatPoints());
  EXPECT_FALSE(Reg.any_of<rogue::StatsBuffComp>(Entity));
  ASSERT_TRUE(Reg.any_of<rogue::StatsBuffPerHitComp>(Entity));

  // First hit
  SBPHC.addStack();
  EXPECT_EQ(SBPHC.Stacks, 1);
  EXPECT_EQ(SBPHC.TicksLeft, 2);
  StatsSystem.update(rogue::System::UpdateType::Tick);
  EXPECT_EQ(Stats.Base, StatPoints);
  EXPECT_EQ(Stats.Bonus, StatPoints);
  EXPECT_TRUE(Reg.any_of<rogue::StatsBuffComp>(Entity));
  ASSERT_TRUE(Reg.any_of<rogue::StatsBuffPerHitComp>(Entity));
  EXPECT_EQ(SBPHC.Stacks, 1);
  EXPECT_EQ(SBPHC.TicksLeft, 1);

  // Second hit
  SBPHC.addStack();
  EXPECT_EQ(SBPHC.Stacks, 2);
  EXPECT_EQ(SBPHC.TicksLeft, 2);
  EXPECT_EQ(SBPHC.getEffectiveBuff(SBPHC.Stacks).Bonus,
            StatPoints + StatPoints);
  EXPECT_EQ(SBPHC.getEffectiveBuff(SBPHC.Stacks).SourceCount, 1);
  StatsSystem.update(rogue::System::UpdateType::Tick);
  EXPECT_EQ(Stats.Bonus, StatPoints + StatPoints);
  ASSERT_TRUE(Reg.any_of<rogue::StatsBuffPerHitComp>(Entity));
  EXPECT_EQ(SBPHC.Stacks, 2);
  EXPECT_EQ(SBPHC.TicksLeft, 1);

  // No hit
  StatsSystem.update(rogue::System::UpdateType::Tick);
  EXPECT_EQ(Stats.Bonus, StatPoints + StatPoints);
  ASSERT_TRUE(Reg.any_of<rogue::StatsBuffPerHitComp>(Entity));
  EXPECT_EQ(SBPHC.Stacks, 2);
  EXPECT_EQ(SBPHC.TicksLeft, 0);

  // Third hit
  SBPHC.addStack();
  EXPECT_EQ(SBPHC.Stacks, 3);
  EXPECT_EQ(SBPHC.TicksLeft, 2);
  StatsSystem.update(rogue::System::UpdateType::Tick);
  EXPECT_EQ(Stats.Bonus, StatPoints + StatPoints + StatPoints);
  ASSERT_TRUE(Reg.any_of<rogue::StatsBuffPerHitComp>(Entity));
  EXPECT_EQ(SBPHC.Stacks, 3);
  EXPECT_EQ(SBPHC.TicksLeft, 1);

  // Fourth hit
  SBPHC.addStack();
  EXPECT_EQ(SBPHC.Stacks, 3);
  EXPECT_EQ(SBPHC.TicksLeft, 2);
  StatsSystem.update(rogue::System::UpdateType::Tick);

  // Nothing x 2
  StatsSystem.update(rogue::System::UpdateType::Tick);
  StatsSystem.update(rogue::System::UpdateType::Tick);

  EXPECT_FALSE(Reg.any_of<rogue::StatsBuffComp>(Entity));
  ASSERT_TRUE(Reg.any_of<rogue::StatsBuffPerHitComp>(Entity));

  // Add a stats buff comp
  auto &SBC = Reg.emplace<rogue::StatsBuffComp>(Entity);
  SBC.Bonus = StatPoints;
  StatsSystem.update(rogue::System::UpdateType::Tick);
  EXPECT_EQ(Stats.Bonus, StatPoints);

  // Fifth hit
  SBPHC.addStack();
  EXPECT_EQ(SBPHC.Stacks, 1);
  EXPECT_EQ(SBPHC.TicksLeft, 2);
  StatsSystem.update(rogue::System::UpdateType::Tick);
  EXPECT_EQ(Stats.Bonus, StatPoints + StatPoints);
  ASSERT_TRUE(Reg.any_of<rogue::StatsBuffPerHitComp>(Entity));
  EXPECT_EQ(SBPHC.Stacks, 1);
  EXPECT_EQ(SBPHC.TicksLeft, 1);

  // Nothing x 2
  StatsSystem.update(rogue::System::UpdateType::Tick);
  StatsSystem.update(rogue::System::UpdateType::Tick);

  EXPECT_TRUE(Reg.any_of<rogue::StatsBuffComp>(Entity));
}

} // namespace