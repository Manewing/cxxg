#include "ItemsCommon.h"

namespace rogue::test {

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

DummyItems::DummyItems()
    : HelmetA(0, "helmet_a", "desc", rogue::ItemType::Helmet, 1,
              {{rogue::CapabilityFlags::Equipment, ArmorEffect},
               {rogue::CapabilityFlags::Dismantle, NullEffect}}),
      HelmetB(0, "helmet_b", "desc", rogue::ItemType::Helmet, 1,
              {{rogue::CapabilityFlags::Equipment, ArmorEffect}}),
      Ring(0, "ring", "desc", rogue::ItemType::Ring, 1,
           {{rogue::CapabilityFlags::Equipment, NullEffect}}),
      // Cursed ring can not be unequipped
      CursedRing(0, "cursed_ring", "desc", rogue::ItemType::Ring, 1,
                 {{rogue::CapabilityFlags::EquipOn, NullEffect}}),
      HealConsumable(0, "dummy_heal", "desc",
                     rogue::ItemType::Consumable | rogue::ItemType::Crafting, 0,
                     {{rogue::CapabilityFlags::UseOn, HealEffect}}),
      PoisonConsumable(0, "poison_liquid", "desc",
                       rogue::ItemType::Consumable | rogue::ItemType::Crafting,
                       5,
                       {{rogue::CapabilityFlags::UseOn, HealEffect},
                        {rogue::CapabilityFlags::UseOn, PoisonEffect}}),
      Potion(0, "potion", "desc",
             rogue::ItemType::Consumable | rogue::ItemType::CraftingBase, 5,
             {{rogue::CapabilityFlags::UseOn, NullEffect}}),
      PlateCrafting(0, "plate", "desc", rogue::ItemType::Crafting, 5,
                    {{rogue::CapabilityFlags::Equipment, ArmorEffect}}),
      CharcoalCrafting(0, "charcoal", "desc", rogue::ItemType::Crafting, 5,
                       {{rogue::CapabilityFlags::UseOn, CleansePoisonEffect}})

{}

ItemDatabase DummyItems::createItemDatabase() {
  ItemDatabase DB;
  std::vector<ItemPrototype *> AllProtos = {
      &this->HelmetA,    &this->HelmetB,        &this->Ring,
      &this->CursedRing, &this->HealConsumable, &this->PoisonConsumable,
      &this->Potion,     &this->PlateCrafting,  &this->CharcoalCrafting};
  for (auto *Proto : AllProtos) {
    Proto->ItemId = DB.getNewItemId();
    DB.addItemProto(*Proto);
  }
  return DB;
}
const std::shared_ptr<rogue::ItemEffect> DummyItems::NullEffect =
    std::make_shared<rogue::NullEffect>();
const std::shared_ptr<rogue::ItemEffect> DummyItems::ArmorEffect =
    std::make_shared<DummyItems::ArmorEffectType>(1);
const std::shared_ptr<rogue::ItemEffect> DummyItems::DamageEffect =
    std::make_shared<DummyItems::DamageEffectType>(1);
const std::shared_ptr<rogue::ItemEffect> DummyItems::HealEffect =
    std::make_shared<DummyItems::HealEffectType>(1);
const std::shared_ptr<rogue::ItemEffect> DummyItems::PoisonEffect =
    std::make_shared<DummyItems::PoisonEffectType>(1);
const std::shared_ptr<rogue::ItemEffect> DummyItems::CleansePoisonEffect =
    std::make_shared<DummyItems::CleansePoisonEffectType>();

} // namespace rogue::test