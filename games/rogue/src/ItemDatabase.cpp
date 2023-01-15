#include <rogue/Components/Buffs.h>
#include <rogue/ItemDatabase.h>

namespace rogue {

template <typename BuffType, typename... RequiredComps>
static std::shared_ptr<ApplyBuffItemEffect<BuffType, RequiredComps...>>
makeApplyBuffItemEffect(const BuffType &Buff) {
  return std::make_shared<ApplyBuffItemEffect<BuffType, RequiredComps...>>(
      Buff);
}

static void fillItemDatabase(ItemDatabase &DB) {
  const int MaxStackSize = 99;

  auto HealEffectx10 = std::make_shared<HealItemEffect>(10);
  auto HealthRegenBuffEffect =
      makeApplyBuffItemEffect<HealthRegenBuffComp, HealthComp>({});
  auto DamageEffectx10 = std::make_shared<DamageItemEffect>(10);
  auto PoisonDebuffEffect =
      makeApplyBuffItemEffect<PoisonDebuffComp, HealthComp>({});
  auto BleeedingDebuffEffect =
      makeApplyBuffItemEffect<BleedingDebuffComp, HealthComp>({});
  auto StatBuffEffect = makeApplyBuffItemEffect<StatsBuffComp, StatsComp>(
      StatsBuffComp{{1U}, StatPoints{1, 1, 1, 10}});

  // FIXME there needs to be a damage buff
  auto StatBuffEffectStr1 = makeApplyBuffItemEffect<StatsBuffComp, StatsComp>(
      StatsBuffComp{{1U}, StatPoints{0, 1, 0, 0}});

  std::vector<ItemPrototype> ItemProtos = {
      ItemPrototype(0, "Berry", ItemType::Consumable, MaxStackSize,
                    {{CapabilityFlags::UseOn, HealEffectx10},
                     {CapabilityFlags::UseOn, HealthRegenBuffEffect}}),

      ItemPrototype(1, "Poison Berry", ItemType::Consumable, MaxStackSize,
                    {{CapabilityFlags::UseOn, DamageEffectx10},
                     {CapabilityFlags::UseOn, PoisonDebuffEffect}}),

      ItemPrototype(2, "Wood", ItemType::Crafting, MaxStackSize, {}),

      ItemPrototype(3, "Stone",
                    ItemType::Weapon | ItemType::Consumable |
                        ItemType::Crafting,
                    MaxStackSize,
                    {{CapabilityFlags::Equipment, StatBuffEffectStr1},
                     {CapabilityFlags::UseOn, BleeedingDebuffEffect},
                     {CapabilityFlags::UseOn, DamageEffectx10}}),

      ItemPrototype(4, "Sword", ItemType::Weapon, 1,
                    {{CapabilityFlags::Equipment, StatBuffEffect},
                     {CapabilityFlags::Equipment, PoisonDebuffEffect}})};

  for (const auto &Proto : ItemProtos) {
    DB.addItemProto(Proto.ItemId, Proto);
  }
}

ItemDatabase::ItemDatabase() { fillItemDatabase(*this); }

void ItemDatabase::addItemProto(int ItemId, const ItemPrototype &ItemProto) {
  // FIXME ID not yet taken
  ItemProtos.emplace(ItemId, ItemProto);
}

Item ItemDatabase::createItem(int ItemId, int StackSize) const {
  const auto &Proto = ItemProtos.at(ItemId);
  return Item(Proto, StackSize);
}

} // namespace rogue