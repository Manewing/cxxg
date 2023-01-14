#include <rogue/Components/Buffs.h>
#include <rogue/ItemDatabase.h>

namespace rogue {

template <typename BuffType, typename... RequiredComps>
static std::shared_ptr<ApplyBuffEffect<BuffType, RequiredComps...>>
makeApplyBuffEffect(const BuffType &Buff) {
  return std::make_shared<ApplyBuffEffect<BuffType, RequiredComps...>>(Buff);
}

static void fillItemDatabase(ItemDatabase &DB) {
  const int MaxStackSize = 99;

  auto HealEffectx10 = std::make_shared<HealItemEffect>(10);
  auto HealthRegenBuffEffect =
      makeApplyBuffEffect<HealthRegenBuffComp, HealthComp>({});
  auto DamageEffectx10 = std::make_shared<DamageItemEffect>(10);
  auto PoisonDebuffEffect =
      makeApplyBuffEffect<PoisonDebuffComp, HealthComp>({});

  std::vector<ItemPrototype> ItemProtos = {
      ItemPrototype(0, "Berry", ItemType::Consumable, MaxStackSize,
                    {HealEffectx10, HealthRegenBuffEffect}),
      ItemPrototype(1, "Poison Berry", ItemType::Consumable, MaxStackSize,
                    {DamageEffectx10, PoisonDebuffEffect}),
      ItemPrototype(2, "Wood", ItemType::Crafting, MaxStackSize, {}),

      ItemPrototype(3, "Stone", ItemType::Crafting, MaxStackSize, {}),

  };

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