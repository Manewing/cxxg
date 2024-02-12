#include <gtest/gtest.h>
#include <rogue/Equipment.h>
#include <rogue/ItemEffect.h>
#include <rogue/ItemPrototype.h>

namespace {

using PId = rogue::ItemProtoId;
const auto NullEffect = std::make_shared<rogue::NullEffect>();

const rogue::ItemPrototype
    DummyConsumable(PId(1), "consumable", "desc", rogue::ItemType::Consumable, 5,
                    {{{rogue::CapabilityFlags::UseOn}, NullEffect}});

const rogue::ItemPrototype
    DummyRing(PId(2), "ring", "desc", rogue::ItemType::Ring, 1,
              {{{rogue::CapabilityFlags::Equipment}, NullEffect}});

TEST(EquipmentSlotTest, Empty) {
  rogue::EquipmentSlot Slot;
  EXPECT_TRUE(Slot.empty());
  EXPECT_EQ(Slot.It, std::nullopt);
  EXPECT_EQ(Slot.BaseTypeFilter, rogue::ItemType::None);
  EXPECT_THROW(Slot.unequip(), std::runtime_error);
}

TEST(EquipmentSlotTest, EquipUnequip) {
  rogue::EquipmentSlot Slot{rogue::ItemType::Ring};
  EXPECT_THROW(Slot.equip(rogue::Item(DummyConsumable)), std::runtime_error);
  Slot.equip(rogue::Item(DummyRing));
  EXPECT_TRUE(Slot.It != std::nullopt);

  auto It = Slot.unequip();
  EXPECT_EQ(It.getName(), "ring");
}

TEST(EquipmentTest, Empty) {
  rogue::Equipment Eq;
  EXPECT_EQ(Eq.all().size(), 8);

  EXPECT_EQ(&Eq.getSlot(rogue::ItemType::Ring), &Eq.Ring);
  EXPECT_EQ(&Eq.getSlot(rogue::ItemType::Amulet), &Eq.Amulet);
  EXPECT_EQ(&Eq.getSlot(rogue::ItemType::Helmet), &Eq.Helmet);
  EXPECT_EQ(&Eq.getSlot(rogue::ItemType::ChestPlate), &Eq.ChestPlate);
  EXPECT_EQ(&Eq.getSlot(rogue::ItemType::Pants), &Eq.Pants);
  EXPECT_EQ(&Eq.getSlot(rogue::ItemType::Boots), &Eq.Boots);
  EXPECT_EQ(&Eq.getSlot(rogue::ItemType::Weapon), &Eq.Weapon);
  EXPECT_EQ(&Eq.getSlot(rogue::ItemType::Ranged), &Eq.OffHand);
  EXPECT_EQ(&Eq.getSlot(rogue::ItemType::Shield), &Eq.OffHand);

  EXPECT_FALSE(Eq.isEquipped(rogue::ItemType::Ring));

  EXPECT_FALSE(Eq.isEquipped(rogue::ItemType::Consumable));
  EXPECT_THROW(Eq.getSlot(rogue::ItemType::Consumable), std::runtime_error);
}

TEST(EquipmentTest, EquipUnquip) {
  entt::registry Reg;
  entt::entity Et = Reg.create();
  rogue::Equipment Eq;

  rogue::Item It(DummyRing);
  EXPECT_TRUE(Eq.canEquip(It, Et, Reg));
  EXPECT_FALSE(Eq.canUnequip(rogue::ItemType::Ring, Et, Reg));

  Eq.equip(It, Et, Reg);
  EXPECT_TRUE(Eq.isEquipped(rogue::ItemType::Ring));
  EXPECT_EQ(Eq.Ring.It->getName(), "ring");
  Eq.unequip(rogue::ItemType::Ring, Et, Reg);
  EXPECT_FALSE(Eq.isEquipped(rogue::ItemType::Ring));
}

} // namespace