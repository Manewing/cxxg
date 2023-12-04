#include <gtest/gtest.h>
#include <rogue/EntityDatabase.h>

namespace {

class DummyEntityAssembler : public rogue::EntityAssembler {
public:
  void assemble(entt::registry &, entt::entity) const override {}
};

TEST(EntityTemplateIdTest, Equality) {
  rogue::EntityTemplateId Id1(0);
  rogue::EntityTemplateId Id2(0);
  rogue::EntityTemplateId Id3(1);

  EXPECT_EQ(Id1, Id2);
  EXPECT_NE(Id1, Id3);
}

TEST(EntityTemplateInfoTest, Inheritance) {
  auto DummyAsm = std::make_shared<DummyEntityAssembler>();

  rogue::EntityTemplateInfo Parent;
  Parent.Name = "parent";
  Parent.DisplayName = "disp_parent";
  Parent.Description = "desc_parent";
  Parent.Id = rogue::EntityTemplateId(0);
  Parent.Assemblers["dummy"] = DummyAsm;

  rogue::EntityTemplateInfo Child;
  Child.Name = "child";
  Child.Id = rogue::EntityTemplateId(1);
  Child.from(Parent);

  EXPECT_EQ(Child.Name, "child");
  EXPECT_EQ(Child.DisplayName, "disp_parent");
  EXPECT_EQ(Child.Description, "desc_parent");
  EXPECT_EQ(Child.Id, rogue::EntityTemplateId(1));
  EXPECT_EQ(Child.Assemblers["dummy"], DummyAsm);

  Child.DisplayName = "disp_child";
  Child.Description = {};
  Child.from(Parent);
  EXPECT_EQ(Child.Name, "child");
  EXPECT_EQ(Child.DisplayName, "disp_child");
  EXPECT_EQ(Child.Description, "desc_parent");
  EXPECT_EQ(Child.Id, rogue::EntityTemplateId(1));

  Child.DisplayName = "disp_child";
  Child.Description = "desc_child";
  Child.from(Parent);
  EXPECT_EQ(Child.Name, "child");
  EXPECT_EQ(Child.DisplayName, "disp_child");
  EXPECT_EQ(Child.Description, "desc_child");
  EXPECT_EQ(Child.Id, rogue::EntityTemplateId(1));
}

TEST(EntityAssemblerCacheTest, Add) {
  rogue::EntityAssemblerCache Cache;
  Cache.add("foo", std::make_shared<DummyEntityAssembler>());

  EXPECT_TRUE(Cache.get("foo"));
  EXPECT_THROW(Cache.get("bar"), std::out_of_range);
}

} // namespace