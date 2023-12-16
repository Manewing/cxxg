#include "ItemsCommon.h"
#include <gtest/gtest.h>
#include <rogue/Components/Buffs.h>
#include <rogue/ItemDatabase.h>
#include <rogue/ItemEffect.h>

namespace rogue {

// After cleanup add this as documentation!
//
// Avoid hard coding specific item types, items are created by the effects
// via composition.
//
//
// How to deal with corner cases such as:
//     Small Sword + Blueberry -> ??
//
//     -> based on capability flags only items with a specific
//         flag already set can be modified with further extensions
//         to that flag
//
//     -> start item determines what elements can be added
//
//     -> utilize item type to avoid allowing items with equip modifier
//         to be equipped if they are not equipment
//
// Arcane Liquid + Glas Vial -> Potion
//
// Rune Dust + Modifier -> Rune
//     Bone -> Strong against undead
//
// Potion + Essence -> Specialized Potion
// -> apply to using entity
//
// Rune + Equipment -> Specialized Equipment
// -> apply to equipping entity
//
// Bomb + Essence -> Specialized Bomb
// -> apply to hit entity
//
// Consumables for:
// -> increased armor (iron skin)
// -> health increase
// -> health generation
// -> special damage (poison, bleeding, frost, etc.)
// -> increased movement speed
// -> block chance
// -> removing debuffs
// -> mana increase
// -> mana generation
//
// Small Sword
// Bone
// Blueberry
// Potion Base
// Sewer Meat
// Charcoal
// Green Goo
// Spider Silk
// Venomous Fang
// Spiderling Venom Sac
//
//
// Crafting tree for recipes
// -> avoid linear lookup of items
// -> map with key tuple (item_id, count)
//
//
// Ideas:
//
// Charcoal: Modifier, Crafting
// - Use: Remove poison debuff
//
// Sewer meat: Modifier, Consumable, Crafting
// - Use: Poison debuff
// - Use: Heals 5HP
//
// Blueberry: Modifier, Consumable, Crafting
// - Use: Health generation increased
//
// Potion Base:  Crafting, PType
// - <emtpy>
//
// Glas vial: Crafting
// - <empt>
//
// Arcane liquid: Crafting
// - <empty>
//
// A valid recipe would be:
// [Glas vial, Arcane liquid]
// It would yield the "Potion Base" item
//
// A valid modifier recipe could be:
// [PType, Sewer meat, Charcoal, Blueberry], "%s Potion"
//
// And would yield for example the following item "Nature Healing Potion":
//
// Nature healing potion:
// - Use: Heals 5 HP
// - Use: Health generation increased
//
// recipes:
//   - ingredients:
//       - name: Leather Scrap
//         count: 10
//     results:
//       - name: Leather
//         count: 1
//   - ingredients:
//       - name: Wooden Log
//         count: 2
//       - name: Leather Scrap
//         count: 1
//       - name: Iron Scrap
//         count: 1
//       - name: Wooden Log
//         count: 2
//     results:
//       - name: Wooden Shield
//         count: 1
//   - ingredients:
//       - name: Rune Dust
//         count: 2
//       - name: Iron Scrap
//         count: 1
//       - name: Stone
//         count: 1
//     results:
//       - name: Fire Stone
//         count: 1
//   - ingredients:
//       - name: Wooden Log
//         count: 5
//       - name: Fire Stone
//         count: 1
//     results:
//       - name: Ash
//         count: 1
//       - name: Fire Stone
//         count: 1
//       - name: Charcoal
//         count: 1
//   - ingredients:
//       - name: Glass Vial
//         count: 1
//       - name: Arcane Liquid
//         count: 1
//     results:
//       - name: Arcane Potion Base
//         count: 1
//
// modifier_recipes:
//   - ingredients:
//       - item_type: potion
//         count: 1
//     modifiers:
//       - item_type: crafting_modifier
//         count: 1
//     results:
//       - template_name: "_X_Potion_Y_"
//         count: 1
//     - ingredients:
//       - item_type: weapon
//         count: 1
//       modifiers:
//       - item_type: crafting_modifier
//         count: 1

class CraftingDatabase {
public:
  CraftingDatabase(const ItemDatabase &Db) : Db(Db) { (void)this->Db; }

private:
  const ItemDatabase &Db;
};

class CraftingSystem {
public:
  CraftingSystem(ItemDatabase &Db) : Db(Db) {}

  std::optional<Item> tryCraft(const std::vector<Item> &Items) {
    if (Items.size() < 2) {
      return std::nullopt;
    }
    auto &First = Items.at(0);
    auto &Second = Items.at(1);

    // If both items are crafting items this indicates a crafting recipe,
    // otherwise it's a modification of an item or invalid combination
    if ((First.getType() & ItemType::Crafting) != ItemType::None &&
        (Second.getType() & ItemType::Crafting) != ItemType::None) {
      // TODO: craft item if it matches recipe,
    }

    // Filter out any invalid combinations
    const auto IsValid =
        ((First.getType() & ItemType::CraftingBase) != ItemType::None &&
         (Second.getType() & ItemType::Crafting) != ItemType::None) ||
        ((First.getType() & ItemType::EquipmentMask) != ItemType::None &&
         (Second.getType() & ItemType::Crafting) != ItemType::None);
    if (!IsValid) {
      return std::nullopt;
    }

    return craftEnhancedItem(Items);
  }

  Item craftEnhancedItem(const std::vector<Item> &Items) {
    auto &First = Items.at(0);
    auto Flags = First.getCapabilityFlags();

    // Create new item prototype, we explicitly copy all effects from the first
    // item (including the specialization effects). The item specialization will
    // not be copied (the first item is already specialized).
    auto NewItemId = Db.getNewItemId();
    ItemPrototype Proto(NewItemId, First.getName(), First.getDescription(),
                        First.getType(), First.getMaxStackSize(),
                        First.getAllEffects());

    // Combine effects from all other items
    for (std::size_t Idx = 1; Idx < Items.size(); ++Idx) {
      auto &Item = Items.at(Idx);
      for (const auto &Info : Item.getAllEffects()) {
        if ((Info.Flags & Flags) != CapabilityFlags::None) {
          Proto.Effects.push_back(Info);
        }
      }
    }

    // Create new effects from effects in prototype that can be added
    std::vector<EffectInfo> NewEffects;
    std::optional<EffectInfo> NullInfo;
    CapabilityFlags EffectFlags = CapabilityFlags::None;
    for (const auto &Info : Proto.Effects) {
      auto Effect = Info.Effect.get();

      // Handle NullEffect separately, we only keep the effect if no other
      // effect has the same flags
      if (dynamic_cast<NullEffect *>(Info.Effect.get())) {
        if (NullInfo) {
          NullInfo->Flags |= Info.Flags;
        } else {
          NullInfo = Info;
        }
        continue;
      }

      // Handle RemoveEffectBase separately, we remove all newly added effects
      // that are removed by this effect
      if (auto *RM = dynamic_cast<const RemoveEffectBase *>(Effect)) {
        auto RMFlags = Info.Flags;
        auto It = std::remove_if(NewEffects.begin(), NewEffects.end(),
                                 [RM, RMFlags](const EffectInfo &Info) {
                                   if (Info.Flags != RMFlags) {
                                     return false;
                                   }
                                   return RM->removesEffect(*Info.Effect);
                                 });
        NewEffects.erase(It, NewEffects.end());
        continue;
      }

      EffectFlags |= Info.Flags;

      for (auto &NewInfo : NewEffects) {
        if (NewInfo.Flags != Info.Flags) {
          continue;
        }
        auto NewEffect = NewInfo.Effect.get();
        if (NewEffect->canAddFrom(*Effect)) {
          NewEffect->addFrom(*Effect);
          Effect = nullptr;
          break;
        }
      }
      if (Effect) {
        NewEffects.push_back({Info.Flags, Effect->clone()});
      }
    }
    if (NullInfo && (NullInfo->Flags & ~EffectFlags) != CapabilityFlags::None) {
      NewEffects.push_back(*NullInfo);
    }
    Proto.Effects = NewEffects;

    // Register the new item prototype
    Db.addItemProto(std::move(Proto));

    // Create the new item
    return Db.createItem(NewItemId, /*StackSize=*/1);
  }

private:
  ItemDatabase &Db;
};

} // namespace rogue

namespace {

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
  rogue::CraftingSystem System(Db);
  auto Result = System.tryCraft({});
  EXPECT_FALSE(Result.has_value());
}

TEST_F(CraftingSystemTest, SingleItem) {
  rogue::CraftingSystem System(Db);
  auto Result = System.tryCraft({Db.createItem(1)});
  EXPECT_FALSE(Result.has_value());
}

TEST_F(CraftingSystemTest, InvalidCombination) {
  rogue::CraftingSystem System(Db);
  auto Helmet = Db.createItem(DummyItems.HelmetA.ItemId);
  auto Ring = Db.createItem(DummyItems.Ring.ItemId);

  auto Result = System.tryCraft({Helmet, Ring});
  EXPECT_FALSE(Result.has_value());
}

TEST_F(CraftingSystemTest, SimpleEquipmentEnhancement) {
  rogue::CraftingSystem System(Db);
  auto Helmet = Db.createItem(DummyItems.HelmetA.ItemId);
  auto Plate = Db.createItem(DummyItems.PlateCrafting.ItemId);
  auto Result = System.tryCraft({Helmet, Plate});
  ASSERT_TRUE(Result.has_value());

  EXPECT_EQ(Result->getType(), rogue::ItemType::Helmet);
  EXPECT_EQ(Result->getName(), "helmet_a");
  EXPECT_EQ(Result->StackSize, 1);
  EXPECT_EQ(Result->getMaxStackSize(), 1);
  EXPECT_EQ(Result->getAllEffects().size(), 2);

  auto ArmorBuff =
      dynamic_cast<const rogue::test::DummyItems::ArmorEffectType *>(
          Result->getAllEffects().at(0).Effect.get());
  ASSERT_NE(ArmorBuff, nullptr);
  EXPECT_EQ(ArmorBuff->Value, 2);
}

TEST_F(CraftingSystemTest, MultiComponentPotionCrafting) {
  rogue::CraftingSystem System(Db);
  auto Potion = Db.createItem(DummyItems.Potion.ItemId);
  auto Poison = Db.createItem(DummyItems.PoisonConsumable.ItemId);
  auto Heal = Db.createItem(DummyItems.HealConsumable.ItemId);

  auto Result = System.tryCraft({Potion, Poison, Heal});
  ASSERT_TRUE(Result.has_value());

  EXPECT_EQ(Result->getType(), rogue::ItemType::Consumable | rogue::ItemType::CraftingBase);
  EXPECT_EQ(Result->getName(), "potion");
  EXPECT_EQ(Result->StackSize, 1);
  EXPECT_EQ(Result->getMaxStackSize(), 5);

  EXPECT_EQ(Result->getAllEffects().size(), 2)
      << "Should be 2, NullEffect needs to be filtered out and HealEffects "
         "should be combined";

  auto HealEffect =
      dynamic_cast<const rogue::test::DummyItems::HealEffectType *>(
          Result->getAllEffects().at(0).Effect.get());
  ASSERT_NE(HealEffect, nullptr);
  EXPECT_EQ(HealEffect->Value, 2);

  auto PoisonEffect =
      dynamic_cast<const rogue::test::DummyItems::PoisonEffectType *>(
          Result->getAllEffects().at(1).Effect.get());
  ASSERT_NE(PoisonEffect, nullptr);
  EXPECT_EQ(PoisonEffect->Value, 1);
}

TEST_F(CraftingSystemTest, RemoveEffectCrafting) {
  rogue::CraftingSystem System(Db);
  auto Potion = Db.createItem(DummyItems.Potion.ItemId);
  auto Poison = Db.createItem(DummyItems.PoisonConsumable.ItemId);
  auto Heal = Db.createItem(DummyItems.HealConsumable.ItemId);
  auto Charcoal = Db.createItem(DummyItems.CharcoalCrafting.ItemId);

  auto Result = System.tryCraft({Potion, Poison, Heal, Charcoal});
  ASSERT_TRUE(Result.has_value());

  EXPECT_EQ(Result->getType(), rogue::ItemType::Consumable | rogue::ItemType::CraftingBase);
  EXPECT_EQ(Result->getName(), "potion");
  EXPECT_EQ(Result->StackSize, 1);
  EXPECT_EQ(Result->getMaxStackSize(), 5);

  EXPECT_EQ(Result->getAllEffects().size(), 1)
      << "Should be 1, NullEffect needs to be filtered out and HealEffects "
         "should be combined, Charcoal should remove the poison effect";

  auto HealEffect =
      dynamic_cast<const rogue::test::DummyItems::HealEffectType *>(
          Result->getAllEffects().at(0).Effect.get());
  ASSERT_NE(HealEffect, nullptr);
  EXPECT_EQ(HealEffect->Value, 2);
}

TEST_F(CraftingSystemTest, InvalidRecipeOnlyCraftingItems) {
  rogue::CraftingSystem System(Db);
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

} // namespace