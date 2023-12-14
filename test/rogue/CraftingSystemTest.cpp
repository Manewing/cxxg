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
        ((First.getType() & ItemType::Consumable) != ItemType::None &&
         (Second.getType() & ItemType::Crafting) != ItemType::None) ||
        ((First.getType() & ItemType::EquipmentMask) != ItemType::None &&
         (Second.getType() & ItemType::Crafting) != ItemType::None);
    if (!IsValid) {
      std::cerr << "not valid: " << getItemTypeLabel(First.getType()) << " +  "
                << getItemTypeLabel(Second.getType()) << std::endl;
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
    for (const auto &Effect : Proto.Effects) {
      // FIXME make this possible for all effects, avoid any duplicate effect
      // types
      auto BuffEff =
          dynamic_cast<const ApplyBuffItemEffectBase *>(Effect.Effect.get());

      if (BuffEff == nullptr) {
        NewEffects.push_back(Effect);
        continue;
      }
      for (auto &NewEffect : NewEffects) {
        if (NewEffect.Flags != Effect.Flags) {
          continue;
        }
        auto NewBuffEff =
            dynamic_cast<ApplyBuffItemEffectBase *>(NewEffect.Effect.get());
        if (NewBuffEff == nullptr) {
          continue;
        }
        if (NewBuffEff->canAddFrom(*BuffEff)) {
          NewBuffEff->addFrom(*BuffEff);
          BuffEff = nullptr;
          break;
        }
      }
      if (BuffEff) {
        NewEffects.push_back({Effect.Flags, BuffEff->clone()});
      }
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

rogue::ArmorBuffComp makeArmorBuffComp(rogue::StatValue MagicArmor,
                                       rogue::StatValue PhysArmor) {
  rogue::ArmorBuffComp Buff;
  Buff.MagicArmor = MagicArmor;
  Buff.PhysArmor = PhysArmor;
  return Buff;
}

rogue::PoisonDebuffComp makePoisonDebuffComp(rogue::StatValue TA,
                                             rogue::StatValue RD, unsigned TP) {
  rogue::PoisonDebuffComp Buff;
  Buff.init(TA, RD, TP);
  return Buff;
}

// FIXME add real null effect type
const auto NullEffect = std::make_shared<rogue::ItemEffect>();
using ArmorEffectType =
    rogue::ApplyBuffItemEffect<rogue::ArmorBuffComp, rogue::HealthComp>;
const auto ArmorEffect =
    rogue::makeApplyBuffItemEffect<rogue::ArmorBuffComp, rogue::HealthComp>(
        makeArmorBuffComp(1, 2));
const auto HealEffect = std::make_shared<rogue::HealItemEffect>(42);
using PoisonEffectType =
    rogue::ApplyBuffItemEffect<rogue::PoisonDebuffComp, rogue::HealthComp>;
const auto PoisonEffect =
    rogue::makeApplyBuffItemEffect<rogue::PoisonDebuffComp, rogue::HealthComp>(
        makePoisonDebuffComp(1.0, 4.0, 2));
// TODO remove poison effect

const rogue::ItemPrototype
    DummyHelmetA(1, "helmet_a", "desc", rogue::ItemType::Helmet, 1,
                 {{rogue::CapabilityFlags::Equipment, ArmorEffect},
                  {rogue::CapabilityFlags::Dismantle, NullEffect}});
const rogue::ItemPrototype
    DummyHelmetB(2, "helmet_b", "desc", rogue::ItemType::Helmet, 1,
                 {{rogue::CapabilityFlags::Equipment, ArmorEffect}});
const rogue::ItemPrototype
    DummyRing(3, "ring", "desc", rogue::ItemType::Ring, 1,
              {{rogue::CapabilityFlags::Equipment, NullEffect}});

// Cursed ring can not be unequipped
const rogue::ItemPrototype
    CursedRing(5, "cursed_ring", "desc", rogue::ItemType::Ring, 1,
               {{rogue::CapabilityFlags::EquipOn, NullEffect}});

const rogue::ItemPrototype
    DummyHeal(6, "dummy_heal", "desc",
              rogue::ItemType::Consumable | rogue::ItemType::Crafting, 5,
              {{rogue::CapabilityFlags::UseOn, HealEffect}});

const rogue::ItemPrototype
    DummyPoison(7, "poison_liquid", "desc",
                rogue::ItemType::Consumable | rogue::ItemType::Crafting, 5,
                {{rogue::CapabilityFlags::UseOn, HealEffect},
                 {rogue::CapabilityFlags::UseOn, PoisonEffect}});

const rogue::ItemPrototype
    DummyPotion(8, "potion", "desc", rogue::ItemType::Consumable, 5,
                {{rogue::CapabilityFlags::UseOn, NullEffect}});

const rogue::ItemPrototype
    DummyPlate(9, "plate", "desc", rogue::ItemType::Crafting, 5,
               {{rogue::CapabilityFlags::Equipment, ArmorEffect}});

rogue::ItemDatabase createDummyDatabase() {
  rogue::ItemDatabase Db;
  Db.addItemProto(DummyHelmetA);
  Db.addItemProto(DummyHelmetB);
  Db.addItemProto(DummyRing);
  Db.addItemProto(CursedRing);
  Db.addItemProto(DummyHeal);
  Db.addItemProto(DummyPoison);
  Db.addItemProto(DummyPotion);
  Db.addItemProto(DummyPlate);
  return Db;
}

class CraftingSystemTest : public ::testing::Test {
public:
  void SetUp() override { Db = createDummyDatabase(); }

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
  auto Helmet = Db.createItem(DummyHelmetA.ItemId);
  auto Ring = Db.createItem(DummyRing.ItemId);

  auto Result = System.tryCraft({Helmet, Ring});
  EXPECT_FALSE(Result.has_value());
}

TEST_F(CraftingSystemTest, SimpleEquipmentEnhancement) {
  rogue::CraftingSystem System(Db);
  auto Helmet = Db.createItem(DummyHelmetA.ItemId);
  auto Plate = Db.createItem(DummyPlate.ItemId);
  auto Result = System.tryCraft({Helmet, Plate});
  ASSERT_TRUE(Result.has_value());

  EXPECT_EQ(Result->getType(), rogue::ItemType::Helmet);
  EXPECT_EQ(Result->getName(), "helmet_a");
  EXPECT_EQ(Result->StackSize, 1);
  EXPECT_EQ(Result->getMaxStackSize(), 1);
  EXPECT_EQ(Result->getAllEffects().size(), 2);

  auto ArmorBuff = dynamic_cast<const ArmorEffectType *>(
      Result->getAllEffects().at(0).Effect.get());
  ASSERT_NE(ArmorBuff, nullptr);
  EXPECT_EQ(ArmorBuff->getBuffCasted().MagicArmor, 2);
  EXPECT_EQ(ArmorBuff->getBuffCasted().PhysArmor, 4);
}

TEST_F(CraftingSystemTest, MultiComponentPotionCrafting) {
  rogue::CraftingSystem System(Db);
  auto Potion = Db.createItem(DummyPotion.ItemId);
  auto Poison = Db.createItem(DummyPoison.ItemId);
  auto Heal = Db.createItem(DummyHeal.ItemId);

  auto Result = System.tryCraft({Potion, Poison, Heal});
  ASSERT_TRUE(Result.has_value());

  EXPECT_EQ(Result->getType(), rogue::ItemType::Consumable);
  EXPECT_EQ(Result->getName(), "potion");
  EXPECT_EQ(Result->StackSize, 1);
  EXPECT_EQ(Result->getMaxStackSize(), 5);

  // FIXME should be two, NullEffect needs to be filtered out
  // and HealEffects should be combined
  EXPECT_EQ(Result->getAllEffects().size(), 4);

  auto HealEffect = dynamic_cast<const rogue::HealItemEffect *>(
      Result->getAllEffects().at(0).Effect.get());
  //  ASSERT_NE(HealEffect, nullptr);
  EXPECT_EQ(HealEffect, nullptr); // FIXME
}

} // namespace