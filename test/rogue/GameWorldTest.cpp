#include <gtest/gtest.h>
#include <rogue/Context.h>
#include <rogue/CreatureDatabase.h>
#include <rogue/LevelDatabase.h>
#include <rogue/GameWorld.h>
#include <rogue/ItemDatabase.h>
#include <rogue/Level.h>
#include <rogue/LevelGenerator.h>

namespace {

class GameWorldTest : public ::testing::Test {
public:
  void SetUp() override {
    ItemDb = rogue::ItemDatabase();
    CreatureDb = rogue::CreatureDatabase();
  }

  rogue::ItemDatabase ItemDb;
  rogue::CreatureDatabase CreatureDb;
  rogue::LevelDatabase LevelDb;
  rogue::GameContext Ctx{ItemDb, CreatureDb, LevelDb};
};

TEST_F(GameWorldTest, MultiLevelDungeonEmpty) {
  rogue::EmptyLevelGenerator LvlGen(Ctx, {{1, 1}});
  rogue::MultiLevelDungeon MLD(LvlGen);

  EXPECT_EQ(MLD.getCurrentLevelIdx(), 0);
  EXPECT_EQ(MLD.getCurrentLevel(), nullptr);
  EXPECT_THROW(MLD.getCurrentLevelOrFail(), std::runtime_error);

  const auto &MLDConst = MLD;
  EXPECT_EQ(MLDConst.getCurrentLevel(), nullptr);
  EXPECT_THROW(MLDConst.getCurrentLevelOrFail(), std::runtime_error);
}

TEST_F(GameWorldTest, MultiLevelDungeonSwitchLevel) {
  rogue::EmptyLevelGenerator LvlGen(Ctx, {{1, 1}});
  rogue::MultiLevelDungeon MLD(LvlGen);

  EXPECT_EQ(MLD.getCurrentLevel(), nullptr);

  MLD.switchLevel(0, true);
  ASSERT_NE(MLD.getCurrentLevel(), nullptr);
  EXPECT_EQ(MLD.getCurrentLevelOrFail().getLevelId(), 0);
  EXPECT_EQ(MLD.getCurrentLevelIdx(), 0);

  MLD.switchLevel(1, true);
  ASSERT_NE(MLD.getCurrentLevel(), nullptr);
  EXPECT_EQ(MLD.getCurrentLevelOrFail().getLevelId(), 1);
  EXPECT_EQ(MLD.getCurrentLevelIdx(), 1);

  // No gaps in levels are allowed when advancing
  EXPECT_THROW(MLD.switchLevel(3, true), std::runtime_error);
  EXPECT_EQ(MLD.getCurrentLevelIdx(), 1);
}

} // namespace