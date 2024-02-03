#include "ItemsCommon.h"

namespace rogue::test {

using CF = rogue::CapabilityFlags;
using IT = rogue::ItemType;

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
    : HelmetA(0, "helmet_a", "desc", IT::Helmet, 1,
              {{{CF::Equipment}, ArmorEffect}, {{CF::Dismantle}, NullEffect}}),
      HelmetB(0, "helmet_b", "desc", IT::Helmet, 1,
              {{{CF::Equipment}, ArmorEffect}}),
      SwordSkillSelf(0, "sword_self", "desc", IT::Weapon, 1,
                     {{{CF::Equipment}, DamageEffect},
                      {{CF::Skill | CF::Self}, SetComp1Effect}}),
      SwordSkillAdjacent(0, "sword_adj", "desc", IT::Weapon, 1,
                         {{{CF::Equipment}, DamageEffect},
                          {{CF::Skill | CF::Adjacent}, SetComp2Effect}}),
      SwordSkillAll(0, "sword_all", "desc", IT::Weapon, 1,
                    {{{CF::Equipment}, DamageEffect},
                     {{CF::Skill | CF::Self}, SetComp1Effect},
                     {{CF::Skill | CF::Adjacent}, SetComp2Effect},
                     {{CF::Skill | CF::Ranged}, SetComp3Effect}}),
      Ring(0, "ring", "desc", IT::Ring, 1, {{{CF::Equipment}, NullEffect}}),
      // Cursed ring can not be unequipped
      CursedRing(0, "cursed_ring", "desc", IT::Ring, 1,
                 {{{CF::EquipOn}, NullEffect}}),
      HealConsumable(0, "dummy_heal", "desc", IT::Consumable | IT::Crafting, 0,
                     {{{CF::UseOn | CF::Self}, HealEffect}}),
      HealThrowConsumable(0, "dummy_heal_throw", "desc",
                          IT::Consumable | IT::Crafting, 0,
                          {{{CF::UseOn | CF::Ranged}, HealEffect}}),
      PoisonConsumable(0, "poison_liquid", "desc",
                       IT::Consumable | IT::Crafting, 5,
                       {{{CF::UseOn | CF::Self}, HealEffect},
                        {{CF::UseOn | CF::Self}, PoisonEffect}}),
      Potion(0, "potion", "desc", IT::Consumable | IT::CraftingBase, 5,
             {{{CF::UseOn | CF::Self}, NullEffect}}),
      PlateCrafting(0, "plate", "desc", IT::Crafting, 5,
                    {{{CF::Equipment}, ArmorEffect}}),
      CharcoalCrafting(0, "charcoal", "desc", IT::Crafting, 5,
                       {{{CF::UseOn | CF::Self}, CleansePoisonEffect}}),
      CraftingA(0, "crafting_a", "desc", IT::Crafting, 5, {{}}),
      CraftingB(0, "crafting_b", "desc", IT::Crafting, 5, {{}}),
      CraftingC(0, "crafting_c", "desc", IT::Crafting, 5, {{}}),
      CraftingD(0, "crafting_d", "desc", IT::Crafting, 5, {{}}) {}

ItemDatabase DummyItems::createItemDatabase() {
  ItemDatabase DB;
  std::vector<ItemPrototype *> AllProtos = {
      &this->HelmetA,    &this->HelmetB,        &this->Ring,
      &this->CursedRing, &this->HealConsumable, &this->PoisonConsumable,
      &this->Potion,     &this->PlateCrafting,  &this->CharcoalCrafting,
      &this->CraftingA,  &this->CraftingB,      &this->CraftingC,
      &this->CraftingD};
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

const std::shared_ptr<rogue::ItemEffect> DummyItems::SetComp1Effect =
    DummyComponentEffect<DummyComp<0>>::get();
const std::shared_ptr<rogue::ItemEffect> DummyItems::SetComp2Effect =
    DummyComponentEffect<DummyComp<1>>::get();
const std::shared_ptr<rogue::ItemEffect> DummyItems::SetComp3Effect =
    DummyComponentEffect<DummyComp<2>>::get();

} // namespace rogue::test