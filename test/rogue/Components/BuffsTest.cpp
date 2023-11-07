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
  std::string getApplyDesc() const override { return ""; }
};

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

} // namespace