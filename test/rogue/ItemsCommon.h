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

  std::string getName() const final {
    return "DummyItemEffect: Name: " + std::to_string(N);
  }

  std::string getDescription() const final {
    return "DummyItemEffect: Desc: " + std::to_string(N);
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

  std::string getName() const final {
    return "DummyComponentEffect: Name: " + std::to_string(CompType::Key);
  }

  std::string getDescription() const final {
    return "DummyComponentEffect: Desc: " + std::to_string(CompType::Key);
  }

  bool canAddFrom(const ItemEffect &Other) const final {
    return dynamic_cast<const OwnType *>(&Other) != nullptr;
  }

  void addFrom(const ItemEffect &Other) final {
    if (auto *OtherDummy = dynamic_cast<const OwnType *>(&Other)) {
      Comp.Value += OtherDummy->Comp.Value;
    }
  }

  bool canApplyTo(const entt::entity &, const entt::entity &DstEt,
                  entt::registry &Reg) const final {
    if constexpr (sizeof...(RequiredComps) == 0) {
      return true;
    } else {
      return Reg.all_of<RequiredComps...>(DstEt);
    }
  }

  void applyTo(const entt::entity &, const entt::entity &DstEt,
               entt::registry &Reg) const final {
    auto *Existing = Reg.try_get<CompType>(DstEt);
    if (Existing) {
      Existing->Value += Comp.Value;
    } else {
      Reg.emplace<CompType>(DstEt, Comp);
    }
  }

  bool canRemoveFrom(const entt::entity &, const entt::entity &DstEt,
                     entt::registry &Reg) const final {
    return Reg.all_of<CompType, RequiredComps...>(DstEt);
  }

  void removeFrom(const entt::entity &, const entt::entity &DstEt,
                  entt::registry &Reg) const final {
    auto *Existing = Reg.try_get<CompType>(DstEt);
    if (Existing) {
      Existing->Value -= Comp.Value;
      if (Existing->Value <= 0) {
        Reg.remove<CompType>(DstEt);
      }
    }
  }

  CompType Comp;
};

template <typename ItemEffectType>
class DummyRemoveEffect : public rogue::RemoveEffect<ItemEffectType> {
public:
  using rogue::RemoveEffect<ItemEffectType>::RemoveEffect;

  std::shared_ptr<ItemEffect> clone() const final {
    return std::make_shared<DummyRemoveEffect<ItemEffectType>>(*this);
  }

  std::string getName() const final {
    return "DummyRemoveEffect: Name: " +
           std::string(typeid(ItemEffectType).name());
  }

  std::string getDescription() const final {
    return "DummyRemoveEffect: Desc: " +
           std::string(typeid(ItemEffectType).name());
  }
};

class DummyItems {
public:
  using ArmorEffectType = DummyItemEffect<0>;
  using DamageEffectType = DummyItemEffect<1>;
  using HealEffectType = DummyItemEffect<2>;
  using PoisonEffectType = DummyItemEffect<3>;
  using CleansePoisonEffectType = DummyRemoveEffect<PoisonEffectType>;

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

  static const std::shared_ptr<ItemEffect> SetComp1Effect;
  static const std::shared_ptr<ItemEffect> SetComp2Effect;
  static const std::shared_ptr<ItemEffect> SetComp3Effect;

  ItemPrototype HelmetA;
  ItemPrototype HelmetB;
  ItemPrototype SwordSkillSelf;
  ItemPrototype SwordSkillAdjacent;
  ItemPrototype SwordSkillAll;
  ItemPrototype Ring;
  ItemPrototype CursedRing;
  ItemPrototype HealConsumable;
  ItemPrototype HealThrowConsumable;
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