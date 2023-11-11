#include <gtest/gtest.h>
#include <rogue/Components/Buffs.h>

namespace {

TEST(BuffsTest, AdditiveBuff) {
  rogue::AdditiveBuff A, B;

  A.SourceCount = 5;
  B.SourceCount = 3;

  A.add(B);

  EXPECT_EQ(A.SourceCount, 8);
  EXPECT_EQ(B.SourceCount, 3);

  A.remove(B);

  EXPECT_EQ(A.SourceCount, 5);
  EXPECT_EQ(B.SourceCount, 3);
}

TEST(BuffsTest, TimedBuffTickExpired) {
  rogue::TimedBuff TB;

  TB.TicksLeft = 0;
  TB.TickPeriod = 0;
  TB.TickPeriodsLeft = 0;
  EXPECT_EQ(TB.totalTicksLeft(), 0);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Expired);
  EXPECT_EQ(TB.totalTicksLeft(), 0);
  EXPECT_FALSE(TB.remove(TB));
}

TEST(BuffsTest, TimedBuffTickWaiting) {
  rogue::TimedBuff TB;

  TB.TicksLeft = 1;
  TB.TickPeriod = 0;
  TB.TickPeriodsLeft = 0;
  EXPECT_EQ(TB.totalTicksLeft(), 1);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Waiting);
  EXPECT_EQ(TB.totalTicksLeft(), 0);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Expired);
  EXPECT_EQ(TB.totalTicksLeft(), 0);
}

TEST(BuffsTest, TimedBuffTickActive) {
  rogue::TimedBuff TB;

  TB.TicksLeft = 0;
  TB.TickPeriod = 1;
  TB.TickPeriodsLeft = 1;
  EXPECT_EQ(TB.totalTicksLeft(), 1);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Active);
  EXPECT_EQ(TB.totalTicksLeft(), 0);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Expired);
  EXPECT_EQ(TB.totalTicksLeft(), 0);

  TB.TicksLeft = 0;
  TB.TickPeriod = 2;
  TB.TickPeriodsLeft = 1;
  EXPECT_EQ(TB.totalTicksLeft(), 2);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Active);
  EXPECT_EQ(TB.totalTicksLeft(), 1);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Waiting);
  EXPECT_EQ(TB.totalTicksLeft(), 0);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Expired);
  EXPECT_EQ(TB.totalTicksLeft(), 0);

  TB.TicksLeft = 0;
  TB.TickPeriod = 3;
  TB.TickPeriodsLeft = 3;
  EXPECT_EQ(TB.totalTicksLeft(), 9);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Active);
  EXPECT_EQ(TB.totalTicksLeft(), 8);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Waiting);
  EXPECT_EQ(TB.totalTicksLeft(), 7);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Waiting);
  EXPECT_EQ(TB.totalTicksLeft(), 6);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Active);
  EXPECT_EQ(TB.totalTicksLeft(), 5);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Waiting);
  EXPECT_EQ(TB.totalTicksLeft(), 4);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Waiting);
  EXPECT_EQ(TB.totalTicksLeft(), 3);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Active);
  EXPECT_EQ(TB.totalTicksLeft(), 2);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Waiting);
  EXPECT_EQ(TB.totalTicksLeft(), 1);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Waiting);
  EXPECT_EQ(TB.totalTicksLeft(), 0);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Expired);
  EXPECT_EQ(TB.totalTicksLeft(), 0);
}

struct TestDiminishingReturnsValueGenBuff
    : public rogue::DiminishingReturnsValueGenBuff {
  std::string getApplyDesc() const override {
    return "Apply: " + getParamApplyDesc("Inc. by ", "<;>", "P");
  }

  std::string getDescription() const {
    return "Description: " + getParamDescription("Inc. by ", "<;>", "P");
  }
};

TEST(BuffsTest, DiminishingReturnsValueGenBuffText) {
  TestDiminishingReturnsValueGenBuff TB;
  TB.init(0.5, 10.0, 2);

  EXPECT_EQ(TB.getApplyDesc(), "Apply: Inc. by 0.5 P<;>");
  EXPECT_EQ(TB.getDescription(),
            "Description: Inc. by 0.5 P every 2 ticks, for 10 ticks (2.5P)<;>");
  TB.init(0.2, 4.0, 2);
}

TEST(BuffsTest, DiminishingReturnsValueGenBuffInit) {
  TestDiminishingReturnsValueGenBuff TB;
  TB.init(0.5, 10.0, 2);
  EXPECT_EQ(TB.TickPeriod, 2);
  EXPECT_EQ(TB.TicksLeft, 0);
  EXPECT_EQ(TB.TickAmount, 0.5);
  EXPECT_EQ(TB.TickPeriodsLeft, 5);
  EXPECT_EQ(TB.RealDuration, 10.0);
  TB.init(0.2, 4.0, 2);
  EXPECT_EQ(TB.TickPeriod, 2);
  EXPECT_EQ(TB.TicksLeft, 0);
  EXPECT_EQ(TB.TickAmount, 0.2);
  EXPECT_EQ(TB.TickPeriodsLeft, 2);
  EXPECT_EQ(TB.RealDuration, 4.0);
}

TEST(BuffsTest, DiminishingReturnsValueGenBuffTick) {
  TestDiminishingReturnsValueGenBuff TB;
  TB.init(0.5, 4.0, 2);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Active);
  EXPECT_EQ(TB.TickPeriod, 2);
  EXPECT_EQ(TB.TicksLeft, 1);
  EXPECT_EQ(TB.TickAmount, 0.5);
  EXPECT_EQ(TB.TickPeriodsLeft, 1);
  EXPECT_EQ(TB.RealDuration, 3.0);
  EXPECT_EQ(TB.totalTicksLeft(), 3);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Waiting);
  EXPECT_EQ(TB.TickPeriod, 2);
  EXPECT_EQ(TB.TicksLeft, 0);
  EXPECT_EQ(TB.TickAmount, 0.5);
  EXPECT_EQ(TB.TickPeriodsLeft, 1);
  EXPECT_EQ(TB.RealDuration, 2.0);
  EXPECT_EQ(TB.totalTicksLeft(), 2);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Active);
  EXPECT_EQ(TB.TickPeriod, 2);
  EXPECT_EQ(TB.TicksLeft, 1);
  EXPECT_EQ(TB.TickAmount, 0.5);
  EXPECT_EQ(TB.TickPeriodsLeft, 0);
  EXPECT_EQ(TB.RealDuration, 1.0);
  EXPECT_EQ(TB.totalTicksLeft(), 1);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Waiting);
  EXPECT_EQ(TB.TickPeriod, 2);
  EXPECT_EQ(TB.TicksLeft, 0);
  EXPECT_EQ(TB.TickAmount, 0.5);
  EXPECT_EQ(TB.TickPeriodsLeft, 0);
  EXPECT_EQ(TB.RealDuration, 0.0);
  EXPECT_EQ(TB.totalTicksLeft(), 0);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Expired);
  EXPECT_EQ(TB.TickPeriod, 2);
  EXPECT_EQ(TB.TicksLeft, 0);
  EXPECT_EQ(TB.TickAmount, 0.5);
  EXPECT_EQ(TB.TickPeriodsLeft, 0);
  EXPECT_EQ(TB.RealDuration, 0.0);
  EXPECT_EQ(TB.totalTicksLeft(), 0);
  EXPECT_EQ(TB.tick(), rogue::TimedBuff::State::Expired);
}

TEST(BuffsTest, DiminishingReturnsValueGenBuffAdd) {
  TestDiminishingReturnsValueGenBuff A, B;
  A.init(0.5, 4.0, 2);
  B.init(0.5, 4.0, 2);
  A.add(B);
  EXPECT_EQ(A.TickPeriod, 2);
  EXPECT_EQ(A.TicksLeft, 0);
  EXPECT_NEAR(A.TickAmount, 0.512, 0.001);
  EXPECT_EQ(A.TickPeriodsLeft, 2);
  EXPECT_NEAR(A.RealDuration, 4.190, 0.001);
  EXPECT_EQ(A.totalTicksLeft(), 4);

  A.init(0.1, 4.0, 2);
  B.init(0.5, 4.0, 2);
  A.add(B);
  EXPECT_EQ(A.TickPeriod, 2);
  EXPECT_EQ(A.TicksLeft, 0);
  EXPECT_NEAR(A.TickAmount, 0.502, 0.001);
  EXPECT_EQ(A.TickPeriodsLeft, 2);
  EXPECT_NEAR(A.RealDuration, 4.190, 0.001);
  EXPECT_EQ(A.totalTicksLeft(), 4);

  A.init(0.5, 4.0, 2);
  B.init(0.1, 8.0, 2);
  A.add(B);
  EXPECT_EQ(A.TickPeriod, 2);
  EXPECT_EQ(A.TicksLeft, 0);
  EXPECT_NEAR(A.TickAmount, 0.502, 0.001);
  EXPECT_EQ(A.TickPeriodsLeft, 4);
  EXPECT_NEAR(A.RealDuration, 8.166, 0.001);
  EXPECT_EQ(A.totalTicksLeft(), 8);

  A.init(0.1, 8.0, 2);
  B.init(0.5, 4.0, 2);
  A.add(B);
  EXPECT_EQ(A.TickPeriod, 2);
  EXPECT_EQ(A.TicksLeft, 0);
  EXPECT_NEAR(A.TickAmount, 0.502, 0.001);
  EXPECT_EQ(A.TickPeriodsLeft, 4);
  EXPECT_NEAR(A.RealDuration, 8.181, 0.001);
  EXPECT_EQ(A.totalTicksLeft(), 8);
}

TEST(BuffsTest, ArmorBuffCompText) {
  rogue::ArmorBuffComp A;
  A.PhysArmor = 5;
  A.MagicArmor = 10;
  EXPECT_EQ(A.getName(), "Armor Buff");
  EXPECT_EQ(A.getDescription(), "5 Armor, 10 Magic Armor");
  A.PhysArmor = 5;
  A.MagicArmor = 0;
  EXPECT_EQ(A.getDescription(), "5 Armor");
  A.PhysArmor = 0;
  A.MagicArmor = 5;
  EXPECT_EQ(A.getDescription(), "5 Magic Armor");
  A.PhysArmor = 0;
  A.MagicArmor = 0;
  EXPECT_EQ(A.getDescription(), "Nothing");
}

TEST(BuffsTest, ArmorBuffCompAddRemove) {
  // Test that both physical armor and magic armor can be added by adding the
  // armor values and increasing the source count, removing reduces the armor
  // and the source count as well.
  //
  // Check that removing a buff with a source count of 0 causes an exception.
  rogue::ArmorBuffComp A, B;
  ASSERT_EQ(A.SourceCount, 1);

  A.PhysArmor = 5;
  A.MagicArmor = 10;
  B.PhysArmor = 3;
  B.MagicArmor = 7;

  A.add(B);
  EXPECT_EQ(A.PhysArmor, 8);
  EXPECT_EQ(A.MagicArmor, 17);
  EXPECT_EQ(A.SourceCount, 2);
  EXPECT_EQ(B.PhysArmor, 3);
  EXPECT_EQ(B.MagicArmor, 7);

  EXPECT_FALSE(A.remove(B));
  EXPECT_EQ(A.PhysArmor, 5);
  EXPECT_EQ(A.MagicArmor, 10);
  EXPECT_EQ(A.SourceCount, 1);
  EXPECT_EQ(B.PhysArmor, 3);
  EXPECT_EQ(B.MagicArmor, 7);

  EXPECT_TRUE(A.remove(A));
  EXPECT_EQ(A.PhysArmor, 0);
  EXPECT_EQ(A.MagicArmor, 0);
  EXPECT_EQ(A.SourceCount, 0);

  // Remove with source count 0 has no effect
  B.SourceCount = 0;
  EXPECT_TRUE(A.remove(B));
  EXPECT_EQ(A.PhysArmor, 0);
  EXPECT_EQ(A.MagicArmor, 0);
  EXPECT_EQ(A.SourceCount, 0);
  B.SourceCount = 1;

  EXPECT_THROW(A.remove(B), std::runtime_error);
}

TEST(BuffsTest, BlockBuffCompText) {
  rogue::BlockBuffComp A;
  A.BlockChance = 5.f;
  EXPECT_EQ(A.getName(), "Chance to block");
  EXPECT_EQ(A.getDescription(), "Chance to block 5%");
}

TEST(BuffsTest, BlockBuffCompAddRemove) {
  // Test that block chance can be added by adding the block chance values and
  // increasing the source count, removing reduces the block chance and the
  // source count as well.
  //
  // Check that removing a buff with a source count of 0 causes an exception.

  rogue::BlockBuffComp A, B;

  A.BlockChance = 5.f;
  B.BlockChance = 3.f;

  A.add(B);
  EXPECT_EQ(A.BlockChance, 8.f);
  EXPECT_EQ(A.SourceCount, 2);
  EXPECT_EQ(B.BlockChance, 3.f);

  EXPECT_FALSE(A.remove(B));
  EXPECT_EQ(A.BlockChance, 5.f);
  EXPECT_EQ(A.SourceCount, 1);
  EXPECT_EQ(B.BlockChance, 3.f);

  EXPECT_TRUE(A.remove(A));
  EXPECT_EQ(A.BlockChance, 0.f);
  EXPECT_EQ(A.SourceCount, 0);

  // Remove with source count 0 has no effect
  B.SourceCount = 0;
  EXPECT_TRUE(A.remove(B));
  EXPECT_EQ(A.BlockChance, 0.f);
  EXPECT_EQ(A.SourceCount, 0);
  B.SourceCount = 1;

  EXPECT_THROW(A.remove(B), std::runtime_error);
}

} // namespace