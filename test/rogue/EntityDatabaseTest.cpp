#include <fstream>
#include <gtest/gtest.h>
#include <rogue/EntityDatabase.h>
#include <rogue/ItemDatabase.h>

namespace {

class DummyEntityAssembler : public rogue::EntityAssembler {
public:
  void assemble(entt::registry &, entt::entity) const override {}
};

class EntityDatabaseTest : public ::testing::Test {
public:
  void SetUp() override {
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

  rogue::EntityDatabase Db;
  rogue::ItemDatabase ItemDb;
};

TEST_F(EntityDatabaseTest, Empty) {
  rogue::EntityDatabase Db;
  EXPECT_TRUE(Db.empty());
  EXPECT_EQ(Db.size(), 0);
  EXPECT_THROW(Db.getEntityTemplate(rogue::EntityTemplateId(0)),
               std::out_of_range);
  const auto &ConstDb = Db;
  EXPECT_THROW(ConstDb.getEntityTemplate(rogue::EntityTemplateId(0)),
               std::out_of_range);
  EXPECT_THROW(Db.getEntityTemplateId("foo"), std::out_of_range);
}

TEST_F(EntityDatabaseTest, AddEntityTemplate) {
  rogue::EntityTemplateInfo Info;
  Info.Name = "foo";
  Info.DisplayName = "disp_foo";
  Info.Description = "desc_foo";
  Info.Id = rogue::EntityTemplateId(42);
  Db.addEntityTemplate(Info);

  EXPECT_FALSE(Db.empty());
  EXPECT_EQ(Db.size(), 1);
  auto Id = Db.getEntityTemplateId("foo");
  EXPECT_EQ(Id, 0);
  const auto &ActInfo = Db.getEntityTemplate(Id);
  EXPECT_EQ(ActInfo.Name, "foo");
  EXPECT_EQ(ActInfo.DisplayName, "disp_foo");
  EXPECT_EQ(ActInfo.Description, "desc_foo");

  EXPECT_EQ(Db.getEntityTemplate(rogue::EntityTemplateId(0)).Id, 0);
  EXPECT_EQ(Db.getEntityTemplate(rogue::EntityTemplateId(0)).Name, "foo");
  EXPECT_EQ(Db.getEntityTemplateId("foo"), 0);
}

TEST_F(EntityDatabaseTest, LoadSimpleEntity) {
  loadDbJson(R"(
  {
    "entity_templates": [
      {
        "name": "foo",
        "display_name": "disp_foo",
        "description": "desc_foo",
        "assemblers": {}
      }
    ]
  }
  )");
  auto Id = Db.getEntityTemplateId("foo");
  EXPECT_EQ(Id, 0);
  const auto &ActInfo = Db.getEntityTemplate(Id);
  EXPECT_EQ(ActInfo.Name, "foo");
  EXPECT_EQ(ActInfo.DisplayName, "disp_foo");
  EXPECT_EQ(ActInfo.Description, "desc_foo");
}

TEST_F(EntityDatabaseTest, LoadEntityWithInheritanceDefaultComps) {
  loadDbJson(R"(
  {
    "entity_templates": [
      {
        "name": "parent",
        "display_name": "disp_parent",
        "description": "desc_parent",
        "assemblers": {
          "collision": true,
          "visible": true
        }
      },
      {
        "name": "child",
        "from": "parent",
        "display_name": "disp_child",
        "assemblers": {
          "visible": false
        }
      },
      {
        "name": "grand_child",
        "from": "child",
        "assemblers": {
          "visible": true,
          "position": true
        }
      }
    ]
  }
  )");
  const auto ParentId = Db.getEntityTemplateId("parent");
  EXPECT_EQ(ParentId, 0);
  const auto &Parent = Db.getEntityTemplate(ParentId);
  EXPECT_EQ(Parent.Name, "parent");
  EXPECT_EQ(Parent.DisplayName, "disp_parent");
  EXPECT_EQ(Parent.Description, "desc_parent");
  EXPECT_TRUE(Parent.Assemblers.count("collision"));
  EXPECT_TRUE(Parent.Assemblers.count("visible"));
  EXPECT_FALSE(Parent.Assemblers.count("position"));

  const auto ChildId = Db.getEntityTemplateId("child");
  EXPECT_EQ(ChildId, 1);
  const auto &Child = Db.getEntityTemplate(ChildId);
  EXPECT_EQ(Child.Name, "child");
  EXPECT_EQ(Child.DisplayName, "disp_child");
  EXPECT_EQ(Child.Description, "desc_parent");
  EXPECT_TRUE(Child.Assemblers.count("collision"));
  EXPECT_FALSE(Child.Assemblers.count("visible"));
  EXPECT_FALSE(Child.Assemblers.count("position"));

  const auto GrandChildId = Db.getEntityTemplateId("grand_child");
  EXPECT_EQ(GrandChildId, 2);
  const auto &GrandChild = Db.getEntityTemplate(GrandChildId);
  EXPECT_EQ(GrandChild.Name, "grand_child");
  EXPECT_EQ(GrandChild.DisplayName, "disp_child");
  EXPECT_EQ(GrandChild.Description, "desc_parent");
  EXPECT_TRUE(GrandChild.Assemblers.count("collision"));
  EXPECT_TRUE(GrandChild.Assemblers.count("visible"));
  EXPECT_TRUE(GrandChild.Assemblers.count("position"));
}

TEST_F(EntityDatabaseTest, LoadEntityWithComplexAssemblers) {
  loadDbJson(R"(
  {
    "entity_templates": [
      {
        "name": "parent",
        "assemblers": {
          "inventory": {
            "loot_table": "dummy_loot_table",
            "is_persistent": true,
            "is_looted": false
          },
          "tile": {
            "char": "f",
            "color": "#ff0000",
            "bg_color": "#0000ff"
          },
          "faction": "nature",
          "race": "undead",
          "equipment": true,
          "auto_equip": true,
          "door": {
            "is_open": false,
            "open_tile": {
              "char": "/",
              "color": "#737373"
            },
            "closed_tile": {
              "char": "+",
              "color": "#737373"
            }
          }
        }
      },
      {
        "name": "child",
        "from": "parent",
        "assemblers": {
          "auto_equip": false,
          "equipment": false,
          "door": false,
          "tile": {
            "char": "c",
            "color": "#00ff00",
            "bg_color": "#0000ff"
          }
        }
      }
    ]
  }
  )");
  const auto ParentId = Db.getEntityTemplateId("parent");
  EXPECT_EQ(ParentId, 0);
  const auto &Parent = Db.getEntityTemplate(ParentId);
  EXPECT_EQ(Parent.Name, "parent");
  EXPECT_TRUE(Parent.Assemblers.count("inventory"));
  EXPECT_TRUE(Parent.Assemblers.count("tile"));
  EXPECT_TRUE(Parent.Assemblers.count("faction"));
  EXPECT_TRUE(Parent.Assemblers.count("auto_equip"));
  EXPECT_TRUE(Parent.Assemblers.count("equipment"));
  EXPECT_TRUE(Parent.Assemblers.count("door"));

  const auto ChildId = Db.getEntityTemplateId("child");
  EXPECT_EQ(ChildId, 1);
  const auto &Child = Db.getEntityTemplate(ChildId);
  EXPECT_EQ(Child.Name, "child");
  EXPECT_TRUE(Child.Assemblers.count("inventory"));
  EXPECT_TRUE(Child.Assemblers.count("tile"));
  EXPECT_TRUE(Child.Assemblers.count("faction"));
  EXPECT_FALSE(Child.Assemblers.count("auto_equip"));
  EXPECT_FALSE(Child.Assemblers.count("equipment"));
  EXPECT_FALSE(Child.Assemblers.count("door"));
}

} // namespace