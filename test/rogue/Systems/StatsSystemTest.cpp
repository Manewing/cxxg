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
  StatsSystem.update(); // Should not affect ref
  EXPECT_EQ(Stats.Base, StatPoints);
  EXPECT_EQ(Stats.Bonus, rogue::StatPoints());

  auto &StatsBuff = Reg.emplace<rogue::StatsBuffComp>(Entity);
  StatsBuff.Bonus = StatPoints;
  StatsSystem.update(); // Should not affect ref
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

  StatsSystem.update();

  EXPECT_EQ(Health.MaxValue, 9.1f * StatPoints.Vit);
  EXPECT_EQ(Health.Value, Health.MaxValue);
  EXPECT_EQ(Mana.MaxValue, 9.1f * StatPoints.Int);
  EXPECT_EQ(Mana.Value, Mana.MaxValue);
  EXPECT_EQ(Agility.Agility, 1.0f * StatPoints.Dex);
}

TEST(TimedStatsSystem, TimedStatSystemStatsTimedBuff) {
  const rogue::StatPoints StatPoints = {1, 2, 3, 4};

  entt::registry Reg;
  rogue::StatsSystem StatsSystem(Reg);
  rogue::TimedStatsSystem TimedStatsSystem(Reg);

  auto Entity = Reg.create();

  auto &Stats = Reg.emplace<rogue::StatsComp>(Entity);
  Stats.Base = StatPoints;
  Stats.Bonus = rogue::StatPoints();

  auto &STBC = Reg.emplace<rogue::StatsTimedBuffComp>(Entity);
  STBC.TicksLeft = 2;
  STBC.Bonus = StatPoints;

  StatsSystem.update();
  TimedStatsSystem.update();
  EXPECT_EQ(Stats.Base, StatPoints);
  EXPECT_EQ(Stats.Bonus, StatPoints);
  ASSERT_TRUE(Reg.any_of<rogue::StatsTimedBuffComp>(Entity));
  EXPECT_EQ(STBC.TicksLeft, 1);

  StatsSystem.update();
  TimedStatsSystem.update();
  EXPECT_EQ(Stats.Base, StatPoints);
  EXPECT_EQ(Stats.Bonus, StatPoints);
  ASSERT_TRUE(Reg.any_of<rogue::StatsTimedBuffComp>(Entity));
  EXPECT_EQ(STBC.TicksLeft, 0);

  StatsSystem.update();
  TimedStatsSystem.update();
  EXPECT_EQ(Stats.Base, StatPoints);
  EXPECT_EQ(Stats.Bonus, rogue::StatPoints());
  EXPECT_FALSE(Reg.any_of<rogue::StatsTimedBuffComp>(Entity));
}

} // namespace