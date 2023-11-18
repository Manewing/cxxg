#include <gtest/gtest.h>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/LOS.h>
#include <rogue/Components/Transform.h>
#include <rogue/Systems/LOSSystem.h>

namespace {

class LOSSystemTest : public ::testing::Test {
public:
  void SetUp() override {
    Reg = entt::registry();
    Sys = std::make_shared<rogue::LOSSystem>(Reg);
    Et = Reg.create();
  }

  entt::registry Reg;
  std::shared_ptr<rogue::LOSSystem> Sys = nullptr;
  entt::entity Et = entt::null;
};

TEST_F(LOSSystemTest, ResetLOS) {
  auto &LOS = Reg.emplace<rogue::LineOfSightComp>(
      Et, rogue::LineOfSightComp{.LOSRange = 10, .MaxLOSRange = 20});
  Reg.emplace<rogue::VisibleLOSComp>(Et).Temporary = true;
  Sys->update(rogue::System::UpdateType::NoTick);

  EXPECT_EQ(LOS.LOSRange, 20);
  EXPECT_EQ(LOS.MaxLOSRange, 20);
  EXPECT_FALSE(Reg.any_of<rogue::VisibleLOSComp>(Et));
}

TEST_F(LOSSystemTest, BlindedDebuff) {
  auto &LOS = Reg.emplace<rogue::LineOfSightComp>(
      Et, rogue::LineOfSightComp{.LOSRange = 100, .MaxLOSRange = 100});
  Reg.emplace<rogue::BlindedDebuffComp>(Et, rogue::BlindedDebuffComp{});

  Sys->update(rogue::System::UpdateType::NoTick);

  EXPECT_EQ(LOS.LOSRange, 10);
  EXPECT_EQ(LOS.MaxLOSRange, 100);
}

TEST_F(LOSSystemTest, MindVisionBuff) {
  Reg.emplace<rogue::LineOfSightComp>(Et);
  Reg.emplace<rogue::VisibleLOSComp>(Et);
  Reg.emplace<rogue::MindVisionBuffComp>(Et, rogue::MindVisionBuffComp{});
  Reg.emplace<rogue::PositionComp>(Et).Pos = {0, 0};

  auto TEt1 = Reg.create();
  Reg.emplace<rogue::LineOfSightComp>(TEt1);
  auto &TPos1 = Reg.emplace<rogue::PositionComp>(TEt1);

  auto TEt2 = Reg.create();
  auto &TPos2 = Reg.emplace<rogue::PositionComp>(TEt2);

  auto TEt3 = Reg.create();
  Reg.emplace<rogue::LineOfSightComp>(TEt3);
  Reg.emplace<rogue::VisibleLOSComp>(TEt3).Temporary = false;
  Reg.emplace<rogue::PositionComp>(TEt3).Pos = {0, 0};

  TPos1.Pos = {0, 0};
  TPos2.Pos = {10, 10};
  Sys->update(rogue::System::UpdateType::NoTick);
  EXPECT_TRUE(Reg.any_of<rogue::VisibleLOSComp>(TEt1));
  EXPECT_FALSE(Reg.any_of<rogue::VisibleLOSComp>(TEt2));
  EXPECT_TRUE(Reg.any_of<rogue::VisibleLOSComp>(TEt3));

  TPos1.Pos = {0, 0};
  TPos2.Pos = {10, 10};
  Reg.emplace<rogue::LineOfSightComp>(TEt2);
  Sys->update(rogue::System::UpdateType::NoTick);
  EXPECT_TRUE(Reg.any_of<rogue::VisibleLOSComp>(TEt1));
  EXPECT_TRUE(Reg.any_of<rogue::VisibleLOSComp>(TEt2));
  EXPECT_TRUE(Reg.any_of<rogue::VisibleLOSComp>(TEt3));

  TPos1.Pos = {100, 100};
  TPos2.Pos = {10, 10};
  Sys->update(rogue::System::UpdateType::NoTick);
  EXPECT_FALSE(Reg.any_of<rogue::VisibleLOSComp>(TEt1));
  EXPECT_TRUE(Reg.any_of<rogue::VisibleLOSComp>(TEt2));
  EXPECT_TRUE(Reg.any_of<rogue::VisibleLOSComp>(TEt3));

  Reg.erase<rogue::MindVisionBuffComp>(Et);
  Sys->update(rogue::System::UpdateType::NoTick);
  EXPECT_FALSE(Reg.any_of<rogue::VisibleLOSComp>(TEt1));
  EXPECT_FALSE(Reg.any_of<rogue::VisibleLOSComp>(TEt2));
  EXPECT_TRUE(Reg.any_of<rogue::VisibleLOSComp>(TEt3));
}

TEST_F(LOSSystemTest, InvisibilityBuff) {
  Reg.emplace<rogue::VisibleComp>(Et).IsVisible = true;
  Reg.emplace<rogue::InvisibilityBuffComp>(Et);

  Sys->update(rogue::System::UpdateType::NoTick);
  EXPECT_FALSE(Reg.get<rogue::VisibleComp>(Et).IsVisible);

  Sys->update(rogue::System::UpdateType::Tick);
  EXPECT_FALSE(Reg.any_of<rogue::InvisibilityBuffComp>(Et));
}

} // namespace