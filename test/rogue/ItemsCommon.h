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

template <unsigned N> struct DummyComp {
  static constexpr auto Key = N;
  int Value = 1;
};

template <unsigned N> struct DummyRequiredComp {
  static constexpr auto Key = N;
  int Value = 1;
};

template <typename CompType, typename... RequiredComps>
class DummyComponentEffect : public ItemEffect {
public:
  using OwnType = DummyComponentEffect<CompType, RequiredComps...>;

public:
  static std::shared_ptr<ItemEffect> get(const CompType &Comp = CompType()) {
    return std::make_shared<OwnType>(Comp);
  }

public:
  explicit DummyComponentEffect(const CompType &Comp) : Comp(Comp) {}

  std::shared_ptr<ItemEffect> clone() const final {
    return std::make_shared<OwnType>(*this);
  }

  bool canAddFrom(const ItemEffect &Other) const final {
    return dynamic_cast<const OwnType *>(&Other) != nullptr;
  }

  void addFrom(const ItemEffect &Other) final {
    if (auto *OtherDummy = dynamic_cast<const OwnType *>(&Other)) {
      Comp.Value += OtherDummy->Comp.Value;
    }
  }

  bool canApplyTo(const entt::entity &Et, entt::registry &Reg) const final {
    if constexpr (sizeof...(RequiredComps) == 0) {
      return true;
    } else {
      return Reg.all_of<RequiredComps...>(Et);
    }
  }

  void applyTo(const entt::entity &Et, entt::registry &Reg) const final {
    auto *Existing = Reg.try_get<CompType>(Et);
    if (Existing) {
      Existing->Value += Comp.Value;
    } else {
      Reg.emplace<CompType>(Et, Comp);
    }
  }

  bool canRemoveFrom(const entt::entity &Et, entt::registry &Reg) const final {
    return Reg.all_of<CompType, RequiredComps...>(Et);
  }

  void removeFrom(const entt::entity &Et, entt::registry &Reg) const final {
    auto *Existing = Reg.try_get<CompType>(Et);
    if (Existing) {
      Existing->Value -= Comp.Value;
      if (Existing->Value <= 0) {
        Reg.remove<CompType>(Et);
      }
    }
  }

  CompType Comp;
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
  ItemPrototype CraftingA;
  ItemPrototype CraftingB;
  ItemPrototype CraftingC;
  ItemPrototype CraftingD;
};

} // namespace rogue::test

#endif // #ifndef ROGUE_TEST_ITEMS_COMMON_H