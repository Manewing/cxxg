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

template <unsigned N>
class DummyItemEffect : public DummyItemEffectBase {
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

public:
  DummyItems();

  ItemDatabase createItemDatabase();

public:
  ItemPrototype DummyHelmetA;
  ItemPrototype DummyHelmetB;
  ItemPrototype DummyRing;
  ItemPrototype DummyCursedRing;
  ItemPrototype DummyHealConsumable;
  ItemPrototype DummyPoisonConsumable;
  ItemPrototype DummyPotion;
  ItemPrototype DummyPlateCrafting;
};

} // namespace rogue::test

#endif // #ifndef ROGUE_TEST_ITEMS_COMMON_H