#include <fstream>
#include <gtest/gtest.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/Transform.h>
#include <rogue/Context.h>
#include <rogue/CreatureDatabase.h>
#include <rogue/ItemDatabase.h>
#include <rogue/LevelDatabase.h>
#include <rogue/LevelGenerator.h>

namespace {

void writeFile(const std::filesystem::path &FilePath,
               const std::string &Content) {
  std::ofstream Out(FilePath);
  if (!Out.is_open()) {
    throw std::runtime_error("Failed to open file " + FilePath.string());
  }
  Out << Content;
}

char getGroundTileKind(const rogue::Level &Lvl, const ymir::Point2d<int> &Pos) {
  return Lvl.Map.get(rogue::Level::LayerGroundIdx).getTile(Pos).kind();
}

char getWallTileKind(const rogue::Level &Lvl, const ymir::Point2d<int> &Pos) {
  return Lvl.Map.get(rogue::Level::LayerWallsIdx).getTile(Pos).kind();
}

class LevelGeneratorTest : public ::testing::Test {
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

TEST_F(LevelGeneratorTest, EmptyLevelGenerator) {
  rogue::EmptyLevelGenerator ELG(Ctx, {ymir::Size2d<int>(1, 1)});
  auto Lvl = ELG.generateLevel(0);
  EXPECT_EQ(Lvl->getLevelId(), 0);
  EXPECT_EQ(Lvl->Map.getSize(), ymir::Size2d<int>(1, 1));
}

TEST_F(LevelGeneratorTest, DesignedMapLevelGeneratorSimpleMap) {
  writeFile("test.map", "####\n"
                        "#  #\n"
                        "#  #\n"
                        "####\n");

  rogue::DesignedMapLevelGenerator::Config Cfg;
  Cfg.MapFile = "test.map";
  Cfg.DefaultChar.T = rogue::Tile{{' '}};
  Cfg.DefaultChar.Layer = "ground";
  Cfg.CharInfoMap['#'].T = rogue::Tile{{'#'}};
  Cfg.CharInfoMap['#'].Layer = "walls";
  Cfg.CharInfoMap[' '] = Cfg.DefaultChar;

  rogue::DesignedMapLevelGenerator DMLG(Ctx, Cfg);

  auto Lvl = DMLG.generateLevel(0);
  EXPECT_EQ(Lvl->getLevelId(), 0);
  EXPECT_EQ(Lvl->Map.getSize(), ymir::Size2d<int>(4, 4));
  EXPECT_EQ(Lvl->Map.get(rogue::Level::LayerWallsIdx)
                .getTile(ymir::Point2d<int>(0, 0)),
            rogue::Tile{{'#'}});
  EXPECT_EQ(Lvl->Map.get(rogue::Level::LayerWallsIdx)
                .getTile(ymir::Point2d<int>(1, 1)),
            rogue::Tile{});
}

TEST_F(LevelGeneratorTest, DesignedMapLevelGeneratorSimpleMapWithEntities) {
  writeFile("test.map", "####\n"
                        "#Cs#\n"
                        "# T#\n"
                        "####\n");

  rogue::DesignedMapLevelGenerator::Config Cfg;
  Cfg.MapFile = "test.map";
  Cfg.DefaultChar.T = rogue::Tile{{' '}};
  Cfg.DefaultChar.Layer = "ground";
  Cfg.CharInfoMap['#'].T = rogue::Tile{{'#'}};
  Cfg.CharInfoMap['#'].Layer = "walls";
  Cfg.CharInfoMap[' '] = Cfg.DefaultChar;
  Cfg.CharInfoMap['s'].T = rogue::Tile{{'s'}};
  Cfg.CharInfoMap['s'].Layer = "entities";
  Cfg.CharInfoMap['T'].T = rogue::Tile{{'T'}};
  Cfg.CharInfoMap['T'].Layer = "entities";
  Cfg.CharInfoMap['C'].T = rogue::Tile{{'C'}};
  Cfg.CharInfoMap['C'].Layer = "entities";
  Cfg.EntityConfig = {.Creatures =
                          {
                              {'s', {"dummy_s", "loot_tb"}},
                              {'T', {"dummy_t", "loot_tb"}},
                          },
                      .Chests =
                          {
                              {'C', {"loot_tb"}},
                          },
                      .Dungeons = {},
                      .LockedDoors = {}};

  ItemDb.addLootTable("loot_tb");
  CreatureDb.addCreature(rogue::CreatureInfo{.Name = "dummy_s"});
  CreatureDb.addCreature(rogue::CreatureInfo{.Name = "dummy_t"});

  rogue::DesignedMapLevelGenerator DMLG(Ctx, Cfg);

  auto Lvl = DMLG.generateLevel(0);
  EXPECT_EQ(Lvl->getLevelId(), 0);
  EXPECT_EQ(Lvl->Map.getSize(), ymir::Size2d<int>(4, 4));
  EXPECT_EQ(getWallTileKind(*Lvl, {0, 0}), '#');

  auto View =
      Lvl->Reg.view<const rogue::PositionComp, const rogue::InventoryComp>();
  EXPECT_EQ(View.size_hint(), 3);
}

TEST_F(LevelGeneratorTest, GeneratedMapLevelGeneratorGenMap) {
  writeFile("test.cfg", "[dungeon]\n"
                        "size: Size2d = {4, 4}\n"
                        "wall: Tile = {'#', \"#534F3C\"}\n"
                        "ground: Tile = {' ', \"#222222\"}\n"
                        "[sequence]\n"
                        " - map_filler\n"
                        "[map_filler]\n"
                        "tile: Tile = {' ', \"#222222\"}\n"
                        "layer = ground\n");

  rogue::GeneratedMapLevelGenerator::Config Cfg;
  Cfg.Seed = 0;
  Cfg.MapConfig = "test.cfg";

  rogue::GeneratedMapLevelGenerator GMLG(Ctx, Cfg);

  auto Lvl = GMLG.generateLevel(0);
  EXPECT_EQ(Lvl->getLevelId(), 0);
  EXPECT_EQ(Lvl->Map.getSize(), ymir::Size2d<int>(4, 4));
  EXPECT_EQ(getWallTileKind(*Lvl, {0, 0}), char(0));
  EXPECT_EQ(getGroundTileKind(*Lvl, {0, 0}), ' ');

  auto View =
      Lvl->Reg.view<const rogue::PositionComp, const rogue::InventoryComp>();
  EXPECT_EQ(View.size_hint(), 0);
}

TEST_F(LevelGeneratorTest, CompositeMultiLevelGeneratorGenLevels) {
  rogue::CompositeMultiLevelGenerator CMG(Ctx);

  const auto A = std::make_shared<rogue::EmptyLevelGenerator>(
      Ctx, rogue::EmptyLevelGenerator::Config{{1, 1}});
  const auto B = std::make_shared<rogue::EmptyLevelGenerator>(
      Ctx, rogue::EmptyLevelGenerator::Config{{2, 2}});
  CMG.addGenerator(A, 0);
  CMG.addGenerator(B, 1);
  CMG.addGenerator(A, 3);
  EXPECT_THROW(CMG.addGenerator(B, 3), std::out_of_range);

  EXPECT_EQ(&CMG.getGeneratorForLevel(0), A.get());
  EXPECT_EQ(&CMG.getGeneratorForLevel(1), B.get());
  EXPECT_EQ(&CMG.getGeneratorForLevel(2), A.get());
  EXPECT_EQ(&CMG.getGeneratorForLevel(3), A.get());
  EXPECT_THROW(CMG.getGeneratorForLevel(4), std::out_of_range);

  auto Lvl0 = CMG.generateLevel(0);
  EXPECT_EQ(Lvl0->getLevelId(), 0);
  EXPECT_EQ(Lvl0->Map.getSize(), ymir::Size2d<int>(1, 1));

  auto Lvl1 = CMG.generateLevel(1);
  EXPECT_EQ(Lvl1->getLevelId(), 1);
  EXPECT_EQ(Lvl1->Map.getSize(), ymir::Size2d<int>(2, 2));

  EXPECT_THROW(CMG.generateLevel(4), std::out_of_range);
}

// TEST(LevelGeneratorTest, ConfigLevelGenerator) {
//   rogue::ConfigLevelGenerator CLG;
// }

} // namespace