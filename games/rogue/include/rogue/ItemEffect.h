#ifndef ROGUE_ITEM_EFFECT_H
#define ROGUE_ITEM_EFFECT_H

#include <entt/entt.hpp>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Stats.h>
#include <rogue/ItemType.h>

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

  virtual bool canApplyTo(const entt::entity &, entt::registry &) const {
    return true;
  }

  virtual void applyTo(const entt::entity &, entt::registry &) const {}

  virtual bool canRemoveFrom(const entt::entity &, entt::registry &) const {
    return true;
  }

  virtual void removeFrom(const entt::entity &, entt::registry &) const {}
};

class HealItemEffect : public ItemEffect {
public:
  explicit HealItemEffect(StatValue Amount);
  StatValue getAmount() const { return Amount; }

  bool canApplyTo(const entt::entity &Et, entt::registry &Reg) const final;
  void applyTo(const entt::entity &Et, entt::registry &Reg) const final;

private:
  StatValue Amount;
};

class DamageItemEffect : public ItemEffect {
public:
  explicit DamageItemEffect(StatValue Amount);
  StatValue getAmount() const { return Amount; }

  bool canApplyTo(const entt::entity &Et, entt::registry &Reg) const final;
  void applyTo(const entt::entity &Et, entt::registry &Reg) const final;

private:
  StatValue Amount;
};

class ApplyBuffItemEffectBase : public ItemEffect {
public:
  virtual const BuffBase &getBuff() const = 0;
};

template <typename BuffType, typename... RequiredComps>
class ApplyBuffItemEffect : public ApplyBuffItemEffectBase {
public:
  explicit ApplyBuffItemEffect(const BuffType &Buff) : Buff(Buff) {}

  const BuffBase &getBuff() const final { return Buff; }

  bool canApplyTo(const entt::entity &Et, entt::registry &Reg) const final {
    return Reg.all_of<RequiredComps...>(Et);
  }

  void applyTo(const entt::entity &Et, entt::registry &Reg) const final {
    auto ExistingBuff = Reg.try_get<BuffType>(Et);
    if (ExistingBuff) {
      ExistingBuff->add(Buff);
    } else {
      Reg.emplace<BuffType>(Et, Buff);
    }
  }

  bool canRemoveFrom(const entt::entity &Et, entt::registry &Reg) const final {
    return Reg.all_of<RequiredComps...>(Et);
  }

  void removeFrom(const entt::entity &Et, entt::registry &Reg) const final {
    auto ExistingBuff = Reg.try_get<BuffType>(Et);
    if (ExistingBuff) {
      if (ExistingBuff->remove(Buff)) {
        Reg.erase<BuffType>(Et);
      }
    }
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