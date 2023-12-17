#ifndef ROGUE_TEST_ITEMS_COMMON_H
#define ROGUE_TEST_ITEMS_COMMON_H

#include <rogue/Item.h>
#include <rogue/ItemDatabase.h>
#include <rogue/ItemEffect.h>

namespace rogue::test {

class DummyItemEffectBase : public ItemEffect {
public:
  explicit DummyItemEffectBase(int Value) : Value(Value) {}
  virtual ~DummyItemEffectBase() = default;

  void addFrom(const ItemEffect &Other) final {
    if (auto *OtherDummy = dynamic_cast<const DummyItemEffectBase *>(&Other)) {
      Value += OtherDummy->Value;
    }
  }

  int Value = 0;
};

template <unsigned N> class DummyItemEffect : public DummyItemEffectBase {
public:
  explicit DummyItemEffect(int Value) : DummyItemEffectBase(Value) {}
  virtual ~DummyItemEffect() = default;

  std::shared_ptr<ItemEffect> clone() const final {
    return std::make_shared<DummyItemEffect<N>>(*this);
  }

  bool canAddFrom(const ItemEffect &Other) const final {
    return dynamic_cast<const DummyItemEffect<N> *>(&Other) != nullptr;
  }
};

class DummyItems {
public:
  using ArmorEffectType = DummyItemEffect<0>;
  using DamageEffectType = DummyItemEffect<1>;
  using HealEffectType = DummyItemEffect<2>;
  using PoisonEffectType = DummyItemEffect<3>;
  using CleansePoisonEffectType = rogue::RemoveEffect<PoisonEffectType>;

public:
  DummyItems();

  ItemDatabase createItemDatabase();

public:
  static const std::shared_ptr<ItemEffect> NullEffect;
  static const std::shared_ptr<ItemEffect> ArmorEffect;
  static const std::shared_ptr<ItemEffect> DamageEffect;
  static const std::shared_ptr<ItemEffect> HealEffect;
  static const std::shared_ptr<ItemEffect> PoisonEffect;
  static const std::shared_ptr<ItemEffect> CleansePoisonEffect;

  ItemPrototype HelmetA;
  ItemPrototype HelmetB;
  ItemPrototype Ring;
  ItemPrototype CursedRing;
  ItemPrototype HealConsumable;
  ItemPrototype PoisonConsumable;
  ItemPrototype Potion;
  ItemPrototype PlateCrafting;
  ItemPrototype CharcoalCrafting;
};

} // namespace rogue::test

#endif // #ifndef ROGUE_TEST_ITEMS_COMMON_H