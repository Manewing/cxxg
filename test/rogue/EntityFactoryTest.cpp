#include <fstream>
#include <gtest/gtest.h>
#include <rogue/Components/Transform.h>
#include <rogue/EntityDatabase.h>
#include <rogue/ItemDatabase.h>

namespace {

class EntityFactoryTest : public ::testing::Test {
public:
  void SetUp() override {
    Reg = entt::registry();
    Db = rogue::EntityDatabase();
    ItemDb = rogue::ItemDatabase();
  }

  void loadDbJson(const std::string &Json) {
    writeFile("test_db.json", Json);
    Db = rogue::EntityDatabase::load(ItemDb, "test_db.json");
  }

  void writeFile(const std::filesystem::path &FilePath,
                 const std::string &Content) {
    std::ofstream Out(FilePath);
    if (!Out.is_open()) {
      throw std::runtime_error("Failed to open file " + FilePath.string());
    }
    Out << Content;
  }

  entt::registry Reg;
  rogue::EntityDatabase Db;
  rogue::ItemDatabase ItemDb;
};

TEST_F(EntityFactoryTest, Empty) {
  rogue::EntityFactory Factory(Reg, Db);
  EXPECT_THROW(Factory.createEntity(rogue::EntityTemplateId(0)),
               std::out_of_range);
}

TEST_F(EntityFactoryTest, CreateEntity) {
  loadDbJson(R"(
    {
      "entity_templates": [
        {
          "name": "foo",
          "display_name": "disp_foo",
          "description": "desc_foo",
          "assemblers": {
            "collision": true
          }
        }
      ]
    }
  )");

  rogue::EntityFactory Factory(Reg, Db);
  auto Entity = Factory.createEntity(Db.getEntityTemplateId("foo"));
  EXPECT_TRUE(Reg.valid(Entity));
  EXPECT_TRUE(Reg.any_of<rogue::CollisionComp>(Entity));
}

} // namespace