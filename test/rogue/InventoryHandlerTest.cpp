#include "ItemsCommon.h"
#include <gtest/gtest.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/CraftingHandler.h>
#include <rogue/Event.h>
#include <rogue/Inventory.h>
#include <rogue/InventoryHandler.h>
#include <rogue/ItemEffect.h>
#include <rogue/ItemPrototype.h>

namespace {

using CId = rogue::CraftingRecipeId;

class InventoryHandlerTest : public ::testing::Test {
public:
  void SetUp() override {
    DummyItems = rogue::test::DummyItems();
    ItemDb = DummyItems.createItemDatabase();
    Crafter = rogue::CraftingHandler(ItemDb);
    Reg = entt::registry();
    Entity = Reg.create();
    Reg.emplace<rogue::NameComp>(Entity, "Entity", "Desc");
  }

  rogue::test::DummyItems DummyItems;
  rogue::ItemDatabase ItemDb;
  rogue::CraftingHandler Crafter;
  entt::registry Reg;
  entt::entity Entity;
};

class EventListener {
public:
  void onPlayerInfoMessageEvent(const rogue::PlayerInfoMessageEvent &E) {
    Messages.push_back(E.Message.str());
  }

  std::vector<std::string> Messages;
};

TEST_F(InventoryHandlerTest, TryUnequipItem) {
  Reg.emplace<rogue::PlayerComp>(Entity);
  Reg.emplace<rogue::HealthComp>(Entity);
  rogue::InventoryHandler InvHandler(Entity, Reg, Crafter);
  rogue::EventHub Hub;
  EventListener Listener;
  Hub.subscribe(Listener, &EventListener::onPlayerInfoMessageEvent);
  InvHandler.setEventHub(&Hub);

  EXPECT_FALSE(InvHandler.tryUnequip(rogue::ItemType::Helmet))
      << "Expect not to be able to unequip if there is not inventory or "
         "equipment";

  auto &Inv = Reg.emplace<rogue::InventoryComp>(Entity).Inv;
  auto &Equip = Reg.emplace<rogue::EquipmentComp>(Entity).Equip;
  InvHandler.refresh();

  EXPECT_FALSE(InvHandler.tryUnequip(rogue::ItemType::Helmet))
      << "Expect not to be able to unequip if there is nothing equipped";

  Equip.equip(rogue::Item(DummyItems.HelmetA), Entity, Reg);
  EXPECT_TRUE(InvHandler.tryUnequip(rogue::ItemType::Helmet))
      << "Expect to be able to unequip if there is something equipped";
  ASSERT_EQ(Inv.size(), 1);
  EXPECT_EQ(Inv.getItem(0).getName(), "helmet_a");
  EXPECT_FALSE(InvHandler.tryUnequip(rogue::ItemType::Helmet));

  Equip.equip(rogue::Item(DummyItems.CursedRing), Entity, Reg);
  EXPECT_FALSE(InvHandler.tryUnequip(rogue::ItemType::Ring))
      << "Expect not to be able to unequip if there is a cursed item equipped";
  ASSERT_EQ(Inv.size(), 1);
  EXPECT_EQ(Inv.getItem(0).getName(), "helmet_a");

  EXPECT_FALSE(InvHandler.tryUnequip(rogue::ItemType::None));

  std::vector<std::string> RefMessages = {
      "Unequip helmet_a from Entity",
      "Can not unequip cursed_ring from Entity"};
  EXPECT_EQ(Listener.Messages, RefMessages);
}

TEST_F(InventoryHandlerTest, TryEquipInventoryItem) {
  Reg.emplace<rogue::PlayerComp>(Entity);
  rogue::InventoryHandler InvHandler(Entity, Reg, Crafter);
  rogue::EventHub Hub;
  EventListener Listener;
  Hub.subscribe(Listener, &EventListener::onPlayerInfoMessageEvent);
  InvHandler.setEventHub(&Hub);

  EXPECT_FALSE(InvHandler.tryEquipItem(0))
      << "Expect not to be able to equip if there is not inventory or "
         "equipment";

  auto &Inv = Reg.emplace<rogue::InventoryComp>(Entity).Inv;
  InvHandler.refresh();
  Inv.addItem(rogue::Item(DummyItems.HelmetA));
  Inv.addItem(rogue::Item(DummyItems.HelmetB));
  Inv.addItem(rogue::Item(DummyItems.Ring));
  Inv.addItem(rogue::Item(DummyItems.CursedRing));
  EXPECT_FALSE(InvHandler.tryEquipItem(0))
      << "Expect not to be able to equip if there is no equipment";

  auto &Equip = Reg.emplace<rogue::EquipmentComp>(Entity).Equip;
  InvHandler.refresh();
  EXPECT_TRUE(Listener.Messages.empty());

  // Try equipping a helmet
  ASSERT_FALSE(Equip.isEquipped(rogue::ItemType::Helmet));
  ASSERT_EQ(Inv.getItem(0).getName(), "helmet_a");
  EXPECT_TRUE(InvHandler.tryEquipItem(0)) << "Expect to be able to equip item";
  ASSERT_EQ(Inv.size(), 3);
  EXPECT_EQ(Inv.getItem(0).getName(), "helmet_b");
  ASSERT_TRUE(Equip.isEquipped(rogue::ItemType::Helmet));
  EXPECT_EQ(Equip.getSlot(rogue::ItemType::Helmet).It->getName(), "helmet_a");

  // Try equipping another helmet
  ASSERT_EQ(Inv.getItem(0).getName(), "helmet_b");
  EXPECT_TRUE(InvHandler.tryEquipItem(0)) << "Expect to be able to equip item";
  ASSERT_EQ(Inv.size(), 3);
  EXPECT_EQ(Inv.getItem(2).getName(), "helmet_a")
      << "Expect old item to be new last item in inventory";
  ASSERT_TRUE(Equip.isEquipped(rogue::ItemType::Helmet));
  EXPECT_EQ(Equip.getSlot(rogue::ItemType::Helmet).It->getName(), "helmet_b");

  // Try equipping a cursed ring
  ASSERT_FALSE(Equip.isEquipped(rogue::ItemType::Ring));
  ASSERT_EQ(Inv.getItem(1).getName(), "cursed_ring");
  EXPECT_TRUE(InvHandler.tryEquipItem(1))
      << "Expect to be able to equip cursed ring";
  ASSERT_EQ(Inv.size(), 2);
  EXPECT_EQ(Inv.getItem(0).getName(), "ring");
  ASSERT_TRUE(Equip.isEquipped(rogue::ItemType::Ring));
  EXPECT_EQ(Equip.getSlot(rogue::ItemType::Ring).It->getName(), "cursed_ring");

  // Try equipping a ring
  ASSERT_EQ(Inv.getItem(0).getName(), "ring");
  EXPECT_FALSE(InvHandler.tryEquipItem(0))
      << "Expect not to be able to equip ring due to equipped cursed ring";
  ASSERT_EQ(Inv.size(), 2);
  EXPECT_EQ(Inv.getItem(0).getName(), "ring");
  ASSERT_TRUE(Equip.isEquipped(rogue::ItemType::Ring));
  EXPECT_EQ(Equip.getSlot(rogue::ItemType::Ring).It->getName(), "cursed_ring");

  EXPECT_THROW(InvHandler.tryEquipItem(2), std::out_of_range)
      << "Expect to throw if trying to equip item index out of range";

  std::vector<std::string> RefMessages = {
      "Equip helmet_a on Entity",
      "Unequip helmet_a from Entity",
      "Equip helmet_b on Entity",
      "Equip cursed_ring on Entity",
      "Can not unequip cursed_ring from Entity",
      "Can not equip ring on Entity"};
  EXPECT_EQ(Listener.Messages, RefMessages);
}

TEST_F(InventoryHandlerTest, TryDropItem) {
  Reg.emplace<rogue::PlayerComp>(Entity);
  rogue::InventoryHandler InvHandler(Entity, Reg, Crafter);
  rogue::EventHub Hub;
  EventListener Listener;
  Hub.subscribe(Listener, &EventListener::onPlayerInfoMessageEvent);
  InvHandler.setEventHub(&Hub);

  EXPECT_FALSE(InvHandler.tryDropItem(0))
      << "Expect not to be able to drop if there is not inventory or position";
  auto &Inv = Reg.emplace<rogue::InventoryComp>(Entity).Inv;
  Reg.emplace<rogue::PositionComp>(Entity);
  InvHandler.refresh();

  EXPECT_THROW(InvHandler.tryDropItem(0), std::out_of_range)
      << "Expect to throw if trying to drop item index out of range";

  Inv.addItem(rogue::Item(DummyItems.HelmetA));

  EXPECT_TRUE(InvHandler.tryDropItem(0))
      << "Expect to be able to drop item if there is inventory and position";

  auto View = Reg.view<rogue::InteractableComp, rogue::InventoryComp>();
  ASSERT_EQ(View.size_hint(), 1)
      << "Expect to create an interactable drop entity when dropping an item";

  std::vector<std::string> RefMessages = {"Entity dropped helmet_a"};
  EXPECT_EQ(Listener.Messages, RefMessages);
}

TEST_F(InventoryHandlerTest, TryDismantleItem) {
  Reg.emplace<rogue::PlayerComp>(Entity);
  rogue::InventoryHandler InvHandler(Entity, Reg, Crafter);
  rogue::EventHub Hub;
  EventListener Listener;
  Hub.subscribe(Listener, &EventListener::onPlayerInfoMessageEvent);
  InvHandler.setEventHub(&Hub);

  EXPECT_FALSE(InvHandler.tryDismantleItem(0))
      << "Expect not to be able to dismantle if there is not inventory or "
         "position";
  auto &Inv = Reg.emplace<rogue::InventoryComp>(Entity).Inv;
  InvHandler.refresh();

  EXPECT_THROW(InvHandler.tryDismantleItem(0), std::out_of_range)
      << "Expect to throw if trying to dismantle item index out of range";

  Inv.addItem(rogue::Item(DummyItems.HelmetA));

  EXPECT_TRUE(InvHandler.tryDismantleItem(0))
      << "Expect to be able to dismantle item if there is inventory and "
         "position";
  ASSERT_EQ(Inv.size(), 0);

  Inv.addItem(rogue::Item(DummyItems.CursedRing));
  EXPECT_FALSE(InvHandler.tryDismantleItem(0))
      << "Expect not to be able to dismantle item if it can not be dismantled";
  ASSERT_EQ(Inv.size(), 1);

  std::vector<std::string> RefMessages = {
      "Dismantled helmet_a for Entity",
      "Cannot dismantle cursed_ring for Entity"};
  EXPECT_EQ(Listener.Messages, RefMessages);
}

TEST_F(InventoryHandlerTest, TryUseItem) {
  Reg.emplace<rogue::PlayerComp>(Entity);
  rogue::InventoryHandler InvHandler(Entity, Reg, Crafter);
  rogue::EventHub Hub;
  EventListener Listener;
  Hub.subscribe(Listener, &EventListener::onPlayerInfoMessageEvent);
  InvHandler.setEventHub(&Hub);

  EXPECT_FALSE(InvHandler.tryUseItem(0))
      << "Expect not to be able to use if there is not inventory or position";
  auto &Inv = Reg.emplace<rogue::InventoryComp>(Entity).Inv;
  InvHandler.refresh();

  EXPECT_THROW(InvHandler.tryUseItem(0), std::out_of_range)
      << "Expect to throw if trying to use item index out of range";

  Inv.addItem(rogue::Item(DummyItems.HealConsumable));

  EXPECT_TRUE(InvHandler.tryUseItem(0))
      << "Expect to be able to use item if there is inventory and position";
  ASSERT_EQ(Inv.size(), 0);

  Inv.addItem(rogue::Item(DummyItems.HelmetA));
  EXPECT_FALSE(InvHandler.tryUseItem(0))
      << "Expect not to be able to use item if it can not be used";

  std::vector<std::string> RefMessages = {
      "Used dummy_heal on Entity", "helmet_a does not have a use effect"};
  EXPECT_EQ(Listener.Messages, RefMessages);
}

TEST_F(InventoryHandlerTest, TryUseItemOnTarget) {
  entt::entity TargetEntity = Reg.create();
  Reg.emplace<rogue::NameComp>(TargetEntity, "Target", "Target");
  Reg.emplace<rogue::PlayerComp>(Entity);
  rogue::InventoryHandler InvHandler(Entity, Reg, Crafter);
  rogue::EventHub Hub;
  EventListener Listener;
  Hub.subscribe(Listener, &EventListener::onPlayerInfoMessageEvent);
  InvHandler.setEventHub(&Hub);

  EXPECT_FALSE(InvHandler.tryUseItemOnTarget(0, TargetEntity))
      << "Expect not to be able to use if there is no inventory";
  auto &Inv = Reg.emplace<rogue::InventoryComp>(Entity).Inv;
  InvHandler.refresh();

  EXPECT_THROW(InvHandler.tryUseItem(0), std::out_of_range)
      << "Expect to throw if trying to use item index out of range";

  Inv.addItem(rogue::Item(DummyItems.HealConsumable));
  EXPECT_FALSE(InvHandler.tryUseItemOnTarget(0, TargetEntity))
      << "Expect not to be able to use item if it is only self use";
  Inv.takeItem(0);
  ASSERT_EQ(Inv.size(), 0);

  Inv.addItem(rogue::Item(DummyItems.HealThrowConsumable));
  EXPECT_TRUE(InvHandler.tryUseItemOnTarget(0, TargetEntity))
      << "Expect to be able to use item if there it can be used on target";
  ASSERT_EQ(Inv.size(), 0);

  Inv.addItem(rogue::Item(DummyItems.HelmetA));
  EXPECT_FALSE(InvHandler.tryUseItemOnTarget(0, TargetEntity))
      << "Expect not to be able to use item if it can not be used";

  std::vector<std::string> RefMessages = {
      "Can not use dummy_heal on Target", "Used dummy_heal_throw on Target",
      "helmet_a does not have a use effect"};
  EXPECT_EQ(Listener.Messages, RefMessages);
}

TEST_F(InventoryHandlerTest, TryUseSkillOnTarget) {
  entt::entity TargetEntity = Reg.create();
  Reg.emplace<rogue::NameComp>(TargetEntity, "Target", "Target");
  Reg.emplace<rogue::PlayerComp>(Entity);

  rogue::InventoryHandler InvHandler(Entity, Reg, Crafter);
  rogue::EventHub Hub;
  EventListener Listener;
  Hub.subscribe(Listener, &EventListener::onPlayerInfoMessageEvent);
  InvHandler.setEventHub(&Hub);

  EXPECT_FALSE(
      InvHandler.tryUseSkillOnTarget(rogue::ItemType::Helmet, TargetEntity))
      << "Expect not to be able to use skill if there is no equipment";
  auto &Equip = Reg.emplace<rogue::EquipmentComp>(Entity).Equip;
  InvHandler.refresh();

  EXPECT_FALSE(
      InvHandler.tryUseSkillOnTarget(rogue::ItemType::Helmet, TargetEntity))
      << "Expect not to be able to use skill if there is nothing equipped";

  Equip.equip(rogue::Item(DummyItems.HelmetA), Entity, Reg);
  EXPECT_FALSE(
      InvHandler.tryUseSkillOnTarget(rogue::ItemType::Helmet, TargetEntity))
      << "Expect not to be able to use skill if the item has no skill";

  Equip.unequip(DummyItems.HelmetA.Type, Entity, Reg);
  Equip.equip(rogue::Item(DummyItems.SwordSkillAdjacent), Entity, Reg);

  EXPECT_TRUE(
      InvHandler.tryUseSkillOnTarget(rogue::ItemType::Weapon, TargetEntity))
      << "Expect to be able to use skill if the item has a skill";
  ASSERT_TRUE(Reg.all_of<rogue::test::DummyComp<1>>(TargetEntity));
  Reg.erase<rogue::test::DummyComp<1>>(TargetEntity);

  Equip.unequip(DummyItems.SwordSkillAdjacent.Type, Entity, Reg);
  Equip.equip(rogue::Item(DummyItems.SwordSkillSelf), Entity, Reg);

  EXPECT_FALSE(
      InvHandler.tryUseSkillOnTarget(rogue::ItemType::Weapon, TargetEntity))
      << "Expect to not be able to use skill if flags are only self";
  EXPECT_FALSE(Reg.any_of<rogue::test::DummyComp<0>>(TargetEntity));

  EXPECT_TRUE(InvHandler.tryUseSkill(rogue::ItemType::Weapon))
      << "Expect to be able to use skill on self";
  ASSERT_TRUE(Reg.all_of<rogue::test::DummyComp<0>>(Entity));
  Reg.erase<rogue::test::DummyComp<0>>(Entity);

  Equip.unequip(DummyItems.SwordSkillSelf.Type, Entity, Reg);
  Equip.equip(rogue::Item(DummyItems.SwordSkillAll), Entity, Reg);

  EXPECT_TRUE(
      InvHandler.tryUseSkillOnTarget(rogue::ItemType::Weapon, TargetEntity))
      << "Expect to be able to use skill if flags are self and adjacent";
  ASSERT_TRUE(Reg.all_of<rogue::test::DummyComp<0>>(Entity));
  EXPECT_FALSE(Reg.any_of<rogue::test::DummyComp<0>>(TargetEntity));
  ASSERT_TRUE(Reg.all_of<rogue::test::DummyComp<1>>(TargetEntity));
  EXPECT_FALSE(Reg.any_of<rogue::test::DummyComp<1>>(Entity));
  Reg.erase<rogue::test::DummyComp<0>>(Entity);
  Reg.erase<rogue::test::DummyComp<1>>(TargetEntity);

  std::vector<std::string> RefMessages = {
      "helmet_a does not have a skill",
      "Used skill DummyComponentEffect: Name: 1 of sword_adj on Target",
      "Can not use skill DummyComponentEffect: Name: 0 of sword_self on Target",
      "Used skill DummyComponentEffect: Name: 0 of sword_self on Entity",
      "Used skill DummyComponentEffect: Name: 0 DummyComponentEffect: Name: 1 "
      "DummyComponentEffect: Name: 2 of sword_all on Target"};
  EXPECT_EQ(Listener.Messages, RefMessages);
}

TEST_F(InventoryHandlerTest, AutoEquipItems) {
  Reg.emplace<rogue::PlayerComp>(Entity);
  auto &Inv = Reg.emplace<rogue::InventoryComp>(Entity).Inv;
  auto &Equip = Reg.emplace<rogue::EquipmentComp>(Entity).Equip;
  rogue::InventoryHandler InvHandler(Entity, Reg, Crafter);
  rogue::EventHub Hub;
  EventListener Listener;
  Hub.subscribe(Listener, &EventListener::onPlayerInfoMessageEvent);
  InvHandler.setEventHub(&Hub);

  Inv.addItem(rogue::Item(DummyItems.HelmetA));
  Inv.addItem(rogue::Item(DummyItems.HelmetB));
  Inv.addItem(rogue::Item(DummyItems.Ring));
  Inv.addItem(rogue::Item(DummyItems.CursedRing));

  InvHandler.autoEquipItems();
  ASSERT_EQ(Inv.size(), 2);
  EXPECT_EQ(Inv.getItem(0).getName(), "helmet_b");
  EXPECT_EQ(Inv.getItem(1).getName(), "cursed_ring");

  ASSERT_TRUE(Equip.isEquipped(rogue::ItemType::Helmet));
  EXPECT_EQ(Equip.getSlot(rogue::ItemType::Helmet).It->getName(), "helmet_a");
  ASSERT_TRUE(Equip.isEquipped(rogue::ItemType::Ring));
  EXPECT_EQ(Equip.getSlot(rogue::ItemType::Ring).It->getName(), "ring");

  std::vector<std::string> RefMessages = {"Equip helmet_a on Entity",
                                          "Equip ring on Entity"};
  EXPECT_EQ(Listener.Messages, RefMessages);
}

TEST_F(InventoryHandlerTest, TryCraftItems) {
  auto &Inv = Reg.emplace<rogue::InventoryComp>(Entity).Inv;
  rogue::InventoryHandler InvHandler(Entity, Reg, Crafter);
  Crafter.addRecipe(
      CId(0),
      rogue::CraftingRecipe(
          "dummy", {DummyItems.CraftingA.ItemId, DummyItems.CraftingB.ItemId},
          {DummyItems.CraftingC.ItemId, DummyItems.CraftingD.ItemId}));

  Inv.addItem(rogue::Item(DummyItems.CraftingA));
  Inv.addItem(rogue::Item(DummyItems.CraftingC));

  InvHandler.tryCraftItems();
  ASSERT_EQ(Inv.size(), 2);
  EXPECT_EQ(Inv.getItem(0).getName(), "crafting_a");
  EXPECT_EQ(Inv.getItem(1).getName(), "crafting_c");

  Inv.takeItem(1);
  Inv.addItem(rogue::Item(DummyItems.CraftingB));

  InvHandler.tryCraftItems();
  ASSERT_EQ(Inv.size(), 2);
  EXPECT_EQ(Inv.getItem(0).getName(), "crafting_c");
  EXPECT_EQ(Inv.getItem(1).getName(), "crafting_d");

  InvHandler.tryCraftItems();
  ASSERT_EQ(Inv.size(), 2);
  EXPECT_EQ(Inv.getItem(0).getName(), "crafting_c");
  EXPECT_EQ(Inv.getItem(1).getName(), "crafting_d");
}

TEST_F(InventoryHandlerTest, TryCraftItemsForPlayer) {
  auto &Inv = Reg.emplace<rogue::InventoryComp>(Entity).Inv;

  auto PlayerEt = Reg.create();
  Reg.emplace<rogue::NameComp>(PlayerEt, "Player", "Desc");
  auto &PC = Reg.emplace<rogue::PlayerComp>(PlayerEt);
  auto &PlayerInv = Reg.emplace<rogue::InventoryComp>(PlayerEt).Inv;

  rogue::InventoryHandler InvHandler(Entity, Reg, Crafter);
  Crafter.addRecipe(
      CId(0),
      rogue::CraftingRecipe(
          "dummy", {DummyItems.CraftingA.ItemId, DummyItems.CraftingB.ItemId},
          {DummyItems.CraftingC.ItemId, DummyItems.CraftingD.ItemId}));

  Inv.addItem(rogue::Item(DummyItems.CraftingA));
  Inv.addItem(rogue::Item(DummyItems.CraftingB));

  InvHandler.tryCraftItems(PlayerEt);
  ASSERT_EQ(Inv.size(), 0);
  ASSERT_EQ(PlayerInv.size(), 2);

  EXPECT_EQ(PlayerInv.getItem(0).getName(), "crafting_c");
  EXPECT_EQ(PlayerInv.getItem(1).getName(), "crafting_d");

  EXPECT_EQ(PC.KnownRecipes, std::set<rogue::CraftingRecipeId>{CId(0)});
}

TEST_F(InventoryHandlerTest, CanCraft) {
  rogue::InventoryHandler InvHandler(Entity, Reg, Crafter);

  auto Recipe = rogue::CraftingRecipe(
      "dummy", {DummyItems.CraftingA.ItemId, DummyItems.CraftingB.ItemId},
      {DummyItems.CraftingC.ItemId});

  EXPECT_FALSE(InvHandler.canCraft(Recipe))
      << "Expect not to be able to craft if there is no inventory";

  auto &Inv = Reg.emplace<rogue::InventoryComp>(Entity).Inv;
  InvHandler.refresh();
  EXPECT_FALSE(InvHandler.canCraft(Recipe))
      << "Expect not to be able to craft if the inventory is empty";

  Inv.addItem(rogue::Item(DummyItems.CraftingA));
  EXPECT_FALSE(InvHandler.canCraft(Recipe))
      << "Expect not to be able to craft if the inventory is missing required "
         "items";

  Inv.addItem(rogue::Item(DummyItems.CraftingB));

  EXPECT_TRUE(InvHandler.canCraft(Recipe))
      << "Expect to be able to craft if the inventory has all required items";
}

TEST_F(InventoryHandlerTest, TryCraft) {
  rogue::InventoryHandler InvHandler(Entity, Reg, Crafter);

  auto Recipe = rogue::CraftingRecipe(
      "dummy", {DummyItems.CraftingA.ItemId, DummyItems.CraftingB.ItemId},
      {DummyItems.CraftingC.ItemId});
  Crafter.addRecipe(CId(0), Recipe);

  EXPECT_FALSE(InvHandler.tryCraft(Recipe))
      << "Expect not to be able to craft if there is no inventory";

  auto &Inv = Reg.emplace<rogue::InventoryComp>(Entity).Inv;
  InvHandler.refresh();
  EXPECT_FALSE(InvHandler.tryCraft(Recipe))
      << "Expect not to be able to craft if the inventory is empty";

  Inv.addItem(rogue::Item(DummyItems.CraftingA));

  EXPECT_FALSE(InvHandler.tryCraft(Recipe))
      << "Expect not to be able to craft if the inventory is missing required "
         "items";

  Inv.addItem(rogue::Item(DummyItems.CraftingB));

  EXPECT_TRUE(InvHandler.tryCraft(Recipe))
      << "Expect to be able to craft if the inventory has all required items";

  ASSERT_EQ(Inv.size(), 1);
  EXPECT_EQ(Inv.getItem(0).getName(), "crafting_c");
}

} // namespace