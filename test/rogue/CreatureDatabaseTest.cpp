#include <gtest/gtest.h>
#include <rogue/CreatureDatabase.h>

namespace {

const rogue::CreatureInfo OrcCreature = {
    "Orc",
    "An orc",
    {11, 12, 13, 14},
    rogue::FactionKind::Enemy,
    rogue::RaceKind::Orc,
};

TEST(CreatureDatabaseTest, addCreature) {
  rogue::CreatureDatabase CDB;
  CDB.addCreature(OrcCreature);
  EXPECT_FALSE(CDB.empty());
  EXPECT_EQ(CDB.size(), 1);
  EXPECT_EQ(CDB.getCreature(rogue::CreatureId(0)).Id, 0);
  EXPECT_EQ(CDB.getCreature(rogue::CreatureId(0)).Name, "Orc");

  EXPECT_THROW(CDB.addCreature(OrcCreature), std::out_of_range);
}

TEST(CreatureDatabaseTest, getCreatureId) {
  rogue::CreatureDatabase CDB;
  CDB.addCreature(OrcCreature);
  EXPECT_EQ(CDB.getCreatureId("Orc"), 0);
}

} // namespace