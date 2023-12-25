#include <fstream>
#include <gtest/gtest.h>
#include <rogue/LevelDatabase.h>

namespace {

TEST(LevelDatabaseTest, LevelInstanceGetInfo) {
  rogue::LevelInstance LI("world_type", "level_cfg.json");
  auto LIInfo = LI.getLevelInfo();
  EXPECT_EQ(LIInfo.WorldType, "world_type");
  EXPECT_EQ(LIInfo.LevelConfig, "level_cfg.json");
}

TEST(LevelDatabaseTest, EmptyLevelTable) {
  rogue::LevelTable LTB;
  EXPECT_THROW(LTB.getLevelInfo(), std::runtime_error);
}

TEST(LevelDatabaseTest, LevelTableGetSlotForRoll) {
  rogue::LevelTable LTB({
      {std::make_shared<rogue::LevelInstance>("a", "b"), 5},
      {std::make_shared<rogue::LevelInstance>("c", "d"), 10},
      {std::make_shared<rogue::LevelInstance>("e", "f"), 20},
  });

  // Sum of weight: 35
  // Rnd Weight: 0-35
  EXPECT_EQ(LTB.getSlotForRoll(0, LTB.getSlots()), 0);
  EXPECT_EQ(LTB.getSlotForRoll(5, LTB.getSlots()), 1);
  EXPECT_EQ(LTB.getSlotForRoll(6, LTB.getSlots()), 1);
  EXPECT_EQ(LTB.getSlotForRoll(12, LTB.getSlots()), 1);
  EXPECT_EQ(LTB.getSlotForRoll(15, LTB.getSlots()), 2);
  EXPECT_EQ(LTB.getSlotForRoll(30, LTB.getSlots()), 2);

  std::srand(0);
  auto LI = LTB.getLevelInfo();
  (void)LI;
}

TEST(LevelDatabaseTest, LevelDatabaseEmpty) {
  rogue::LevelDatabase Db;
  EXPECT_THROW(Db.getLevelInfo("foo"), std::out_of_range);
}

TEST(LevelDatabaseTest, LevelDatabaseAddLevelInstance) {
  rogue::LevelDatabase Db;
  auto LTB = std::make_shared<rogue::LevelTable>();
  LTB->reset({
      {std::make_shared<rogue::LevelInstance>("a", "b"), 5},
  });
  Db.addLevelTable("l1", LTB);
  auto LI = Db.getLevelInfo("l1");
  EXPECT_EQ(LI.WorldType, "a");
  EXPECT_EQ(LI.LevelConfig, "b");
  EXPECT_THROW(Db.addLevelTable("l1", LTB), std::out_of_range);
}

TEST(LevelDatabaseTest, LevelDatabaseLoad) {
  std::filesystem::path LevelDbConfig = "level_db.json";
  std::ofstream OutStream(LevelDbConfig);
  ASSERT_TRUE(OutStream.is_open());
  OutStream << R"(
  {
    "level_tables": {
      "level_1": {
        "slots": [
          {
            "type": "level",
            "game_world": "multi_level_dungeon",
            "level_config": "levels/sewers.json",
            "weight": 5
          }
        ]
      },
      "level_2": {
        "slots": [
          {
            "type": "table",
            "ref": "level_1",
            "weight": 5
          },
          {
            "type": "level",
            "game_world": "multi_level_dungeon",
            "level_config": "levels/rat_pit.json",
            "weight": 50
          }
        ]
      }
    }
  }
  )";
  OutStream.close();

  rogue::LevelDatabase Db = rogue::LevelDatabase::load(LevelDbConfig);
  std::srand(0);
  auto LI = Db.getLevelInfo("level_1");
  EXPECT_EQ(LI.WorldType, "multi_level_dungeon");
  EXPECT_EQ(LI.LevelConfig, "levels/sewers.json");
  LI = Db.getLevelInfo("level_2");
  EXPECT_EQ(LI.WorldType, "multi_level_dungeon");
  EXPECT_EQ(LI.LevelConfig, "levels/rat_pit.json");
}

} // namespace