#include "ItemsCommon.h"
#include <gtest/gtest.h>
#include <rogue/Components/Buffs.h>
#include <rogue/CraftingHandler.h>
#include <rogue/ItemDatabase.h>
#include <rogue/ItemEffect.h>

namespace {

using CId = rogue::CraftingRecipeId;
using PId = rogue::ItemProtoId;

class CraftingSystemTest : public ::testing::Test {
public:
  void SetUp() override {
    DummyItems = rogue::test::DummyItems();
    Db = DummyItems.createItemDatabase();
  }

  rogue::test::DummyItems DummyItems;
  rogue::ItemDatabase Db;
};

TEST_F(CraftingSystemTest, Empty) {
  rogue::CraftingHandler System(Db);
  auto Result = System.tryCraft({});
  EXPECT_FALSE(Result.has_value());
}

TEST_F(CraftingSystemTest, SingleItem) {
  rogue::CraftingHandler System(Db);
  auto Result = System.tryCraft({Db.createItem(PId(1))});
  EXPECT_FALSE(Result.has_value());
}

TEST_F(CraftingSystemTest, InvalidCombination) {
  rogue::CraftingHandler System(Db);
  auto Helmet = Db.createItem(DummyItems.HelmetA.ItemId);
  auto Ring = Db.createItem(DummyItems.Ring.ItemId);

  auto Result = System.tryCraft({Helmet, Ring});
  EXPECT_FALSE(Result.has_value());
}

TEST_F(CraftingSystemTest, SimpleEquipmentEnhancement) {
  rogue::CraftingHandler System(Db);
  auto Helmet = Db.createItem(DummyItems.HelmetA.ItemId);
  auto Plate = Db.createItem(DummyItems.PlateCrafting.ItemId);

  auto ResultVec = System.tryCraft({Helmet, Plate});
  ASSERT_TRUE(ResultVec.has_value());
  ASSERT_EQ(ResultVec->size(), 1);
  const auto &Result = ResultVec->at(0);

  EXPECT_EQ(Result.getType(), rogue::ItemType::Helmet);
  EXPECT_EQ(Result.getName(), "helmet_a");
  EXPECT_EQ(Result.StackSize, 1);
  EXPECT_EQ(Result.getMaxStackSize(), 1);
  EXPECT_EQ(Result.getAllEffects().size(), 2);

  auto ArmorBuff =
      dynamic_cast<const rogue::test::DummyItems::ArmorEffectType *>(
          Result.getAllEffects().at(0).Effect.get());
  ASSERT_NE(ArmorBuff, nullptr);
  EXPECT_EQ(ArmorBuff->Value, 2);
}

TEST_F(CraftingSystemTest, CapabilityMismatchEnhancement) {
  rogue::CraftingHandler System(Db);
  auto Potion = Db.createItem(DummyItems.Potion.ItemId);
  auto Plate = Db.createItem(DummyItems.PlateCrafting.ItemId);

  auto ResultVec = System.tryCraft({Potion, Plate});
  ASSERT_TRUE(ResultVec.has_value());
  ASSERT_EQ(ResultVec->size(), 1);
  const auto &Result = ResultVec->at(0);

  EXPECT_EQ(Result.getType(), DummyItems.Potion.Type);
  EXPECT_EQ(Result.getName(), "potion");
  EXPECT_EQ(Result.StackSize, 1);
  EXPECT_EQ(Result.getMaxStackSize(), 5);
  ASSERT_EQ(Result.getAllEffects().size(), 1);
  EXPECT_EQ(Result.getAllEffects().at(0).Effect.get(),
            DummyItems.NullEffect.get());
}

TEST_F(CraftingSystemTest, MultiComponentPotionCrafting) {
  rogue::CraftingHandler System(Db);
  auto Potion = Db.createItem(DummyItems.Potion.ItemId);
  auto Poison = Db.createItem(DummyItems.PoisonConsumable.ItemId);
  auto Heal = Db.createItem(DummyItems.HealConsumable.ItemId);

  auto ResultVec = System.tryCraft({Potion, Poison, Heal});
  ASSERT_TRUE(ResultVec.has_value());
  ASSERT_EQ(ResultVec->size(), 1);
  const auto &Result = ResultVec->at(0);

  EXPECT_EQ(Result.getType(),
            rogue::ItemType::Consumable | rogue::ItemType::CraftingBase);
  EXPECT_EQ(Result.getName(), "potion");
  EXPECT_EQ(Result.StackSize, 1);
  EXPECT_EQ(Result.getMaxStackSize(), 5);

  EXPECT_EQ(Result.getAllEffects().size(), 2)
      << "Should be 2, NullEffect needs to be filtered out and HealEffects "
         "should be combined";

  auto HealEffect =
      dynamic_cast<const rogue::test::DummyItems::HealEffectType *>(
          Result.getAllEffects().at(0).Effect.get());
  ASSERT_NE(HealEffect, nullptr);
  EXPECT_EQ(HealEffect->Value, 2);

  auto PoisonEffect =
      dynamic_cast<const rogue::test::DummyItems::PoisonEffectType *>(
          Result.getAllEffects().at(1).Effect.get());
  ASSERT_NE(PoisonEffect, nullptr);
  EXPECT_EQ(PoisonEffect->Value, 1);
}

TEST_F(CraftingSystemTest, RemoveEffectCrafting) {
  rogue::CraftingHandler System(Db);
  auto Potion = Db.createItem(DummyItems.Potion.ItemId);
  auto Poison = Db.createItem(DummyItems.PoisonConsumable.ItemId);
  auto Heal = Db.createItem(DummyItems.HealConsumable.ItemId);
  auto Charcoal = Db.createItem(DummyItems.CharcoalCrafting.ItemId);

  auto ResultVec = System.tryCraft({Potion, Poison, Heal, Charcoal});
  ASSERT_TRUE(ResultVec.has_value());
  ASSERT_EQ(ResultVec->size(), 1);
  const auto &Result = ResultVec->at(0);

  EXPECT_EQ(Result.getType(),
            rogue::ItemType::Consumable | rogue::ItemType::CraftingBase);
  EXPECT_EQ(Result.getName(), "potion");
  EXPECT_EQ(Result.StackSize, 1);
  EXPECT_EQ(Result.getMaxStackSize(), 5);

  EXPECT_EQ(Result.getAllEffects().size(), 1)
      << "Should be 1, NullEffect needs to be filtered out and HealEffects "
         "should be combined, Charcoal should remove the poison effect";

  auto HealEffect =
      dynamic_cast<const rogue::test::DummyItems::HealEffectType *>(
          Result.getAllEffects().at(0).Effect.get());
  ASSERT_NE(HealEffect, nullptr);
  EXPECT_EQ(HealEffect->Value, 2);
}

TEST_F(CraftingSystemTest, InvalidRecipeOnlyCraftingItems) {
  rogue::CraftingHandler System(Db);
  auto Plate = Db.createItem(DummyItems.PlateCrafting.ItemId);
  auto Charcoal = Db.createItem(DummyItems.CharcoalCrafting.ItemId);
  auto Heal = Db.createItem(DummyItems.HealConsumable.ItemId);

  auto Result = System.tryCraft({Plate, Charcoal});
  EXPECT_FALSE(Result.has_value());

  Result = System.tryCraft({Heal, Charcoal});
  EXPECT_FALSE(Result.has_value());

  Result = System.tryCraft({Charcoal, Charcoal});
  EXPECT_FALSE(Result.has_value());

  Result = System.tryCraft({Plate, Charcoal, Heal});
  EXPECT_FALSE(Result.has_value());
}

TEST_F(CraftingSystemTest, SimpleRecipe) {
  rogue::CraftingHandler System(Db);
  rogue::CraftingRecipe Recipe(
      "dummy", {DummyItems.CraftingA.ItemId, DummyItems.CraftingB.ItemId},
      {DummyItems.CraftingC.ItemId});
  System.addRecipe(CId(0), Recipe);

  auto A = Db.createItem(DummyItems.CraftingA.ItemId);
  auto B = Db.createItem(DummyItems.CraftingB.ItemId);

  auto ResultVec = System.tryCraft({A, B});
  ASSERT_TRUE(ResultVec.has_value());
  ASSERT_EQ(ResultVec->size(), 1);
  const auto &Result = ResultVec->at(0);
  EXPECT_EQ(Result.getName(), "crafting_c");
}

TEST_F(CraftingSystemTest, RecipeOverrides) {
  rogue::CraftingHandler System(Db);
  auto Potion = Db.createItem(DummyItems.Potion.ItemId);
  auto Heal = Db.createItem(DummyItems.HealConsumable.ItemId);

  auto ResultVec = System.tryCraft({Potion, Heal});
  ASSERT_TRUE(ResultVec.has_value());
  ASSERT_EQ(ResultVec->size(), 1);
  EXPECT_EQ(ResultVec->at(0).getName(), "potion");

  rogue::CraftingRecipe Recipe(
      "dummy", {DummyItems.Potion.ItemId, DummyItems.HealConsumable.ItemId},
      {DummyItems.CraftingA.ItemId});
  System.addRecipe(CId(0), Recipe);

  ResultVec = System.tryCraft({Potion, Heal});
  ASSERT_TRUE(ResultVec.has_value());
  ASSERT_EQ(ResultVec->size(), 1);
  EXPECT_EQ(ResultVec->at(0).getName(), "crafting_a");
}

TEST_F(CraftingSystemTest, MultipleRecipes) {
  rogue::CraftingHandler System(Db);
  rogue::CraftingRecipe RecipeAB(
      "dummy_ab", {DummyItems.CraftingA.ItemId, DummyItems.CraftingB.ItemId},
      {DummyItems.HelmetA.ItemId});
  rogue::CraftingRecipe RecipeABC("dummy_abc",
                                  {DummyItems.CraftingA.ItemId,
                                   DummyItems.CraftingB.ItemId,
                                   DummyItems.CraftingC.ItemId},
                                  {DummyItems.HelmetB.ItemId});
  rogue::CraftingRecipe RecipeAC(
      "dummy_ac", {DummyItems.CraftingA.ItemId, DummyItems.CraftingC.ItemId},
      {DummyItems.Potion.ItemId});
  System.addRecipe(CId(0), RecipeAB);
  System.addRecipe(CId(1), RecipeABC);
  System.addRecipe(CId(2), RecipeAC);

  auto A = Db.createItem(DummyItems.CraftingA.ItemId);
  auto B = Db.createItem(DummyItems.CraftingB.ItemId);
  auto C = Db.createItem(DummyItems.CraftingC.ItemId);

  auto ResultVec = System.tryCraft({A, B});
  ASSERT_TRUE(ResultVec.has_value());
  ASSERT_EQ(ResultVec->size(), 1);
  EXPECT_EQ(ResultVec->at(0).getName(), "helmet_a");

  ResultVec = System.tryCraft({A, B, C});
  ASSERT_TRUE(ResultVec.has_value());
  ASSERT_EQ(ResultVec->size(), 1);
  EXPECT_EQ(ResultVec->at(0).getName(), "helmet_b");

  ResultVec = System.tryCraft({A, C});
  ASSERT_TRUE(ResultVec.has_value());
  ASSERT_EQ(ResultVec->size(), 1);
  EXPECT_EQ(ResultVec->at(0).getName(), "potion");
}

TEST_F(CraftingSystemTest, NoItemDb) {
  rogue::CraftingHandler System;
  auto A = Db.createItem(DummyItems.CraftingA.ItemId);
  auto B = Db.createItem(DummyItems.CraftingB.ItemId);
  auto ResultVec = System.tryCraft({A, B});
  EXPECT_FALSE(ResultVec.has_value());
}

} // namespace