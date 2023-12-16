#ifndef ROGUE_ITEM_EFFECT_H
#define ROGUE_ITEM_EFFECT_H

#include <entt/entt.hpp>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Stats.h>
#include <rogue/ItemType.h>

namespace rogue {
class ItemDatabase;
}

namespace rogue {

// Add durability to items:
//  Durability
//  MaxDurability
// Both get reduced by being used, MaxDurability way slower than durability
// If durability is zero item breaks
// Durability can be restored to MaxDurability by repairing
// MaxDurability can only be restored with rare special items

class ItemEffect {
public:
  virtual ~ItemEffect() = default;

  virtual std::shared_ptr<ItemEffect> clone() const = 0;

  virtual bool canAddFrom(const ItemEffect &) const { return true; }

  virtual void addFrom(const ItemEffect &) {}

  virtual bool canApplyTo(const entt::entity &, entt::registry &) const {
    return true;
  }

  virtual void applyTo(const entt::entity &, entt::registry &) const {}

  virtual bool canRemoveFrom(const entt::entity &, entt::registry &) const {
    return true;
  }

  virtual void removeFrom(const entt::entity &, entt::registry &) const {}
};

class NullEffect : public ItemEffect {
public:
  std::shared_ptr<ItemEffect> clone() const final;
};

class HealItemEffect : public ItemEffect {
public:
  explicit HealItemEffect(StatValue Amount);
  StatValue getAmount() const { return Amount; }

  std::shared_ptr<ItemEffect> clone() const final;
  bool canAddFrom(const ItemEffect &Other) const final;
  void addFrom(const ItemEffect &Other) final;

  bool canApplyTo(const entt::entity &Et, entt::registry &Reg) const final;
  void applyTo(const entt::entity &Et, entt::registry &Reg) const final;

private:
  StatValue Amount;
};

class DamageItemEffect : public ItemEffect {
public:
  explicit DamageItemEffect(StatValue Amount);
  StatValue getAmount() const { return Amount; }

  std::shared_ptr<ItemEffect> clone() const final;
  bool canAddFrom(const ItemEffect &Other) const final;
  void addFrom(const ItemEffect &Other) final;
  bool canApplyTo(const entt::entity &Et, entt::registry &Reg) const final;
  void applyTo(const entt::entity &Et, entt::registry &Reg) const final;

private:
  StatValue Amount;
};

class DismantleEffect : public ItemEffect {
public:
  struct DismantleResult {
    int ItemId = -1;
    unsigned Amount = 0;
  };

public:
  explicit DismantleEffect(const ItemDatabase &ItemDb,
                           std::vector<DismantleResult> Results);
  std::shared_ptr<ItemEffect> clone() const final;
  bool canAddFrom(const ItemEffect &Other) const final;
  void addFrom(const ItemEffect &Other) final;
  bool canApplyTo(const entt::entity &Et, entt::registry &Reg) const final;
  void applyTo(const entt::entity &Et, entt::registry &Reg) const final;

private:
  const ItemDatabase &ItemDb;
  std::vector<DismantleResult> Results;
};

// FIXME add remove component effect

template <typename CompType, typename... RequiredComps>
class SetComponentEffect : public ItemEffect {
public:
  using OwnType = SetComponentEffect<CompType, RequiredComps...>;

public:
  static const CompType *getOrNull(const ItemEffect &It) {
    if (auto *SCE = dynamic_cast<const OwnType *>(&It)) {
      return &SCE->Comp;
    }
    return nullptr;
  }

public:
  explicit SetComponentEffect(const CompType &Comp) : Comp(Comp) {}

  std::shared_ptr<ItemEffect> clone() const final {
    return std::make_shared<OwnType>(Comp);
  }

  /// Adding is possible if the item effect has the identical type
  bool canAddFrom(const ItemEffect &Other) const final {
    return dynamic_cast<const OwnType *>(&Other) != nullptr;
  }

  /// Adding has no effect, only the first component setting effect is kept
  void addFrom(const ItemEffect &) final {}

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
      *Existing = Comp;
    } else {
      Reg.emplace<CompType>(Et, Comp);
    }
  }

  bool canRemoveFrom(const entt::entity &Et, entt::registry &Reg) const final {
    if constexpr (sizeof...(RequiredComps) == 0) {
      return true;
    } else {
      return Reg.all_of<RequiredComps...>(Et);
    }
  }

  void removeFrom(const entt::entity &Et, entt::registry &Reg) const final {
    Reg.remove<CompType>(Et);
  }

private:
  CompType Comp;
};

template <typename CompType, typename... RequiredComps>
static std::shared_ptr<SetComponentEffect<CompType, RequiredComps...>>
makeSetComponentEffect(const CompType &Comp) {
  return std::make_shared<SetComponentEffect<CompType, RequiredComps...>>(Comp);
}

class ApplyBuffItemEffectBase : public ItemEffect {
public:
  virtual const BuffBase &getBuff() const = 0;
};

template <typename BuffType, typename... RequiredComps>
class ApplyBuffItemEffect : public ApplyBuffItemEffectBase {
public:
  using OwnType = ApplyBuffItemEffect<BuffType, RequiredComps...>;

public:
  explicit ApplyBuffItemEffect(const BuffType &Buff) : Buff(Buff) {}

  const BuffBase &getBuff() const final { return Buff; }

  const BuffType &getBuffCasted() const { return Buff; }

  bool canApplyTo(const entt::entity &Et, entt::registry &Reg) const final {
    return BuffApplyHelper<BuffType, RequiredComps...>::canApplyTo(Et, Reg);
  }

  void applyTo(const entt::entity &Et, entt::registry &Reg) const final {
    BuffApplyHelper<BuffType, RequiredComps...>::applyTo(Buff, Et, Reg);
  }

  bool canRemoveFrom(const entt::entity &Et, entt::registry &Reg) const final {
    return BuffApplyHelper<BuffType, RequiredComps...>::canRemoveFrom(Et, Reg);
  }

  void removeFrom(const entt::entity &Et, entt::registry &Reg) const final {
    BuffApplyHelper<BuffType, RequiredComps...>::removeFrom(Buff, Et, Reg);
  }

  std::shared_ptr<ItemEffect> clone() const final {
    return std::make_shared<OwnType>(static_cast<const OwnType &>(*this));
  }

  bool canAddFrom(const ItemEffect &Other) const final {
    return dynamic_cast<const OwnType *>(&Other) != nullptr;
  }

  void addFrom(const ItemEffect &Other) final {
    auto *OtherPtr = dynamic_cast<const OwnType *>(&Other);
    assert(OtherPtr);
    Buff.add(OtherPtr->Buff);
  }

private:
  BuffType Buff;
};

template <typename BuffType, typename... RequiredComps>
static std::shared_ptr<ApplyBuffItemEffect<BuffType, RequiredComps...>>
makeApplyBuffItemEffect(const BuffType &Buff) {
  return std::make_shared<ApplyBuffItemEffect<BuffType, RequiredComps...>>(
      Buff);
}

} // namespace rogue

#endif // ROGUE_ITEM_EFFECT_H