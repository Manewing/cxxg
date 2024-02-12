#include "ItemsCommon.h"

namespace rogue::test {

using CF = rogue::CapabilityFlags;
using IT = rogue::ItemType;
using Id = rogue::ItemProtoId;

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
    : HelmetA(Id(0), "helmet_a", "desc", IT::Helmet, 1,
              {{{CF::Equipment}, ArmorEffect}, {{CF::Dismantle}, NullEffect}}),
      HelmetB(Id(0), "helmet_b", "desc", IT::Helmet, 1,
              {{{CF::Equipment}, ArmorEffect}}),
      ChestPlateNoEffects(Id(0), "chest_plate_no_effects", "desc",
                          IT::ChestPlate, 1, {{}}),
      Sword(Id(0), "sword", "desc", IT::Weapon, 1,
            {{{CF::Skill | CF::Self}, NullEffect}}),
      RuneSpellAdjacent(Id(0), "rune_spell_adj", "desc",
                        IT::Crafting | IT::Quest, 1,
                        {{{CF::Skill | CF::Adjacent}, DamageEffect}}),
      SwordSkillSelf(Id(0), "sword_self", "desc", IT::Weapon, 1,
                     {{{CF::Equipment}, DamageEffect},
                      {{CF::Skill | CF::Self}, SetComp1Effect}}),
      SwordSkillAdjacent(Id(0), "sword_adj", "desc", IT::Weapon, 1,
                         {{{CF::Equipment}, DamageEffect},
                          {{CF::Skill | CF::Adjacent}, SetComp2Effect}}),
      SwordSkillAll(Id(0), "sword_all", "desc", IT::Weapon, 1,
                    {{{CF::Equipment}, DamageEffect},
                     {{CF::Skill | CF::Self}, SetComp1Effect},
                     {{CF::Skill | CF::Adjacent}, SetComp2Effect},
                     {{CF::Skill | CF::Ranged}, SetComp3Effect}}),
      Ring(Id(0), "ring", "desc", IT::Ring, 1, {{{CF::Equipment}, NullEffect}}),
      // Cursed ring can not be unequipped
      CursedRing(Id(0), "cursed_ring", "desc", IT::Ring, 1,
                 {{{CF::EquipOn}, NullEffect}}),
      HealConsumable(Id(0), "dummy_heal", "desc", IT::Consumable | IT::Crafting,
                     5, {{{CF::UseOn | CF::Self}, HealEffect}}),
      HealThrowConsumable(Id(0), "dummy_heal_throw", "desc",
                          IT::Consumable | IT::Crafting, 5,
                          {{{CF::UseOn | CF::Ranged}, HealEffect}}),
      PoisonConsumable(Id(0), "poison_liquid", "desc",
                       IT::Consumable | IT::Crafting, 5,
                       {{{CF::UseOn | CF::Self}, HealEffect},
                        {{CF::UseOn | CF::Self}, PoisonEffect}}),
      Potion(Id(0), "potion", "desc", IT::Consumable | IT::CraftingBase, 5,
             {{{CF::UseOn | CF::Self}, NullEffect}}),
      PlateCrafting(Id(0), "plate", "desc", IT::Crafting, 5,
                    {{{CF::Equipment}, ArmorEffect}}, ItemType::ArmorMask),
      CharcoalCrafting(Id(0), "charcoal", "desc", IT::Crafting, 5,
                       {{{CF::UseOn | CF::Self}, CleansePoisonEffect}}),
      CraftingA(Id(0), "crafting_a", "desc", IT::Crafting, 5, {{}}),
      CraftingB(Id(0), "crafting_b", "desc", IT::Crafting, 5, {{}}),
      CraftingC(Id(0), "crafting_c", "desc", IT::Crafting, 5, {{}}),
      CraftingD(Id(0), "crafting_d", "desc", IT::Crafting, 5, {{}}) {}

ItemDatabase DummyItems::createItemDatabase() {
  ItemDatabase DB;
  std::vector<ItemPrototype *> AllProtos = {&this->HelmetA,
                                            &this->HelmetB,
                                            &this->ChestPlateNoEffects,
                                            &this->Sword,
                                            &this->RuneSpellAdjacent,
                                            &this->SwordSkillSelf,
                                            &this->SwordSkillAdjacent,
                                            &this->SwordSkillAll,
                                            &this->Ring,
                                            &this->CursedRing,
                                            &this->HealConsumable,
                                            &this->PoisonConsumable,
                                            &this->Potion,
                                            &this->PlateCrafting,
                                            &this->CharcoalCrafting,
                                            &this->CraftingA,
                                            &this->CraftingB,
                                            &this->CraftingC,
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