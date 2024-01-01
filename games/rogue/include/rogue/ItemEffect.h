#ifndef ROGUE_ITEM_EFFECT_H
#define ROGUE_ITEM_EFFECT_H

#include <entt/entt.hpp>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Stats.h>
#include <rogue/ItemType.h>

namespace rogue {
class ItemDatabase;
struct BuffBase;
} // namespace rogue

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

  virtual std::string getName() const = 0;
  virtual std::string getDescription() const = 0;

  virtual bool canAddFrom(const ItemEffect &) const { return true; }

  virtual void addFrom(const ItemEffect &) {}

  /// Checks if the effect can be applied to the given target entity
  /// @param SrcEt The entity that applies the effect
  /// @param DstEt The entity to apply the effect to
  /// @param Reg The registry the entities belong to
  virtual bool canApplyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                          entt::registry &Reg) const {
    (void)SrcEt;
    (void)DstEt;
    (void)Reg;
    return true;
  }

  /// Applies the effect to the given target entity
  /// @param SrcEt The entity that applies the effect
  /// @param DstEt The entity to apply the effect to
  /// @param Reg The registry the entities belong to
  virtual void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                       entt::registry &Reg) const {
    (void)SrcEt;
    (void)DstEt;
    (void)Reg;
  }

  /// Checks if the effect can be removed from the given target entity
  /// @param SrcEt The entity that removes the effect
  /// @param DstEt The entity to remove the effect from
  /// @param Reg The registry the entities belong to
  virtual bool canRemoveFrom(const entt::entity &SrcEt,
                             const entt::entity &DstEt,
                             entt::registry &Reg) const {
    (void)SrcEt;
    (void)DstEt;
    (void)Reg;
    return true;
  }

  /// Removes the effect from the given target entity
  /// @param SrcEt The entity that removes the effect
  /// @param DstEt The entity to remove the effect from
  /// @param Reg The registry the entities belong to
  virtual void removeFrom(const entt::entity &SrcEt, const entt::entity &DstEt,
                          entt::registry &Reg) const {
    (void)SrcEt;
    (void)DstEt;
    (void)Reg;
  }

  /// Marks the entities as in combat
  static void markCombat(const entt::entity &SrcEt, const entt::entity &DstEt,
                         entt::registry &Reg);
};

class NullEffect : public ItemEffect {
public:
  std::shared_ptr<ItemEffect> clone() const final;
  std::string getName() const final;
  std::string getDescription() const final;
};

class HealItemEffect : public ItemEffect {
public:
  explicit HealItemEffect(StatValue Amount);
  StatValue getAmount() const { return Amount; }

  std::shared_ptr<ItemEffect> clone() const final;
  std::string getName() const final;
  std::string getDescription() const final;

  bool canAddFrom(const ItemEffect &Other) const final;
  void addFrom(const ItemEffect &Other) final;

  bool canApplyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg) const final;

  void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
               entt::registry &Reg) const final;

private:
  StatValue Amount;
};

class DamageItemEffect : public ItemEffect {
public:
  explicit DamageItemEffect(StatValue Amount);
  StatValue getAmount() const { return Amount; }

  std::shared_ptr<ItemEffect> clone() const final;
  std::string getName() const final;
  std::string getDescription() const final;

  bool canAddFrom(const ItemEffect &Other) const final;
  void addFrom(const ItemEffect &Other) final;
  bool canApplyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg) const final;
  void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
               entt::registry &Reg) const final;

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
  std::string getName() const final;
  std::string getDescription() const final;

  bool canAddFrom(const ItemEffect &Other) const final;
  void addFrom(const ItemEffect &Other) final;
  bool canApplyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg) const final;
  void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
               entt::registry &Reg) const final;

private:
  const ItemDatabase &ItemDb;
  std::vector<DismantleResult> Results;
};

template <typename CompType, bool IsCombat, typename... RequiredComps>
class SetComponentEffect : public ItemEffect {
public:
  using OwnType = SetComponentEffect<CompType, IsCombat, RequiredComps...>;

public:
  static const CompType *getOrNull(const ItemEffect &It) {
    if (auto *SCE = dynamic_cast<const OwnType *>(&It)) {
      return &SCE->Comp;
    }
    return nullptr;
  }

public:
  SetComponentEffect() = default;
  explicit SetComponentEffect(const CompType &Comp) : Comp(Comp) {}

  /// Adding is possible if the item effect has the identical type
  bool canAddFrom(const ItemEffect &Other) const final {
    return dynamic_cast<const OwnType *>(&Other) != nullptr;
  }

  /// Adding has no effect, only the first component setting effect is kept
  void addFrom(const ItemEffect &) final {}

  bool canApplyTo(const entt::entity &, const entt::entity &DstEt,
                  entt::registry &Reg) const final {
    if constexpr (sizeof...(RequiredComps) == 0) {
      return true;
    } else {
      return Reg.all_of<RequiredComps...>(DstEt);
    }
  }

  void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
               entt::registry &Reg) const final {
    auto *Existing = Reg.try_get<CompType>(DstEt);
    if (Existing) {
      *Existing = Comp;
    } else {
      Reg.emplace<CompType>(DstEt, Comp);
    }
    if constexpr (IsCombat) {
      markCombat(SrcEt, DstEt, Reg);
    }
  }

  bool canRemoveFrom(const entt::entity &, const entt::entity &DstEt,
                     entt::registry &Reg) const final {
    if constexpr (sizeof...(RequiredComps) == 0) {
      return true;
    } else {
      return Reg.all_of<RequiredComps...>(DstEt);
    }
  }

  void removeFrom(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg) const final {
    Reg.remove<CompType>(DstEt);
    if constexpr (IsCombat) {
      markCombat(SrcEt, DstEt, Reg);
    }
  }

protected:
  CompType Comp;
};

class ApplyBuffItemEffectBase : public ItemEffect {
public:
  virtual const BuffBase &getBuff() const = 0;
  std::string getName() const final;
  std::string getDescription() const final;
};

template <typename BuffType, bool IsCombat, typename... RequiredComps>
class ApplyBuffItemEffect : public ApplyBuffItemEffectBase {
public:
  using OwnType = ApplyBuffItemEffect<BuffType, IsCombat, RequiredComps...>;
  using Helper = BuffApplyHelper<BuffType, IsCombat, RequiredComps...>;

public:
  explicit ApplyBuffItemEffect(const BuffType &Buff) : Buff(Buff) {}

  const BuffBase &getBuff() const final { return Buff; }

  const BuffType &getBuffCasted() const { return Buff; }

  bool canApplyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg) const final {
    return Helper::canApplyTo(SrcEt, DstEt, Reg);
  }

  void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
               entt::registry &Reg) const final {
    Helper::applyTo(Buff, SrcEt, DstEt, Reg);
  }

  bool canRemoveFrom(const entt::entity &SrcEt, const entt::entity &DstEt,
                     entt::registry &Reg) const final {
    return Helper::canRemoveFrom(SrcEt, DstEt, Reg);
  }

  void removeFrom(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg) const final {
    Helper::removeFrom(Buff, SrcEt, DstEt, Reg);
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

class RemoveEffectBase : public ItemEffect {
public:
  virtual bool removesEffect(const ItemEffect &Other) const = 0;
};

template <typename ItemEffectType>
class RemoveEffect : public RemoveEffectBase {
public:
  using OwnType = RemoveEffect<ItemEffectType>;

public:
  /// Adding is possible if the item effect has the identical type. However it
  /// has no effect, only the first component removing the effect is kept
  bool canAddFrom(const ItemEffect &Other) const final {
    return dynamic_cast<const OwnType *>(&Other) != nullptr;
  }

  /// Removes effect of matching type
  bool removesEffect(const ItemEffect &Other) const final {
    return dynamic_cast<const ItemEffectType *>(&Other) != nullptr;
  }
};

template <typename CompType, bool IsCombat, typename... RequiredComps>
class RemoveComponentEffect : public ItemEffect {
public:
  using OwnType = RemoveComponentEffect<CompType, IsCombat, RequiredComps...>;

public:
  explicit RemoveComponentEffect() {}

  /// Adding is possible if the item effect has the identical type. However it
  /// has no effect, only the first component removing the effect is kept
  bool canAddFrom(const ItemEffect &Other) const final {
    return dynamic_cast<const OwnType *>(&Other) != nullptr;
  }

  bool canApplyTo(const entt::entity &, const entt::entity &DstEt,
                  entt::registry &Reg) const final {
    if constexpr (sizeof...(RequiredComps) == 0) {
      return true;
    } else {
      return Reg.all_of<CompType, RequiredComps...>(DstEt);
    }
  }

  void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
               entt::registry &Reg) const final {
    Reg.remove<CompType>(DstEt);
    if constexpr (IsCombat) {
      markCombat(SrcEt, DstEt, Reg);
    }
  }

  /// Removing the effect does nothing
  bool canRemoveFrom(const entt::entity &, const entt::entity &,
                     entt::registry &) const final {
    return true;
  }

  void removeFrom(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg) const final {
    if constexpr (IsCombat) {
      markCombat(SrcEt, DstEt, Reg);
    }
  }
};

} // namespace rogue

#endif // ROGUE_ITEM_EFFECT_H