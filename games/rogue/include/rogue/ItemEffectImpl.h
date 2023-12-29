#ifndef ROGUE_ITEM_EFFECT_IMPL_H
#define ROGUE_ITEM_EFFECT_IMPL_H

#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Stats.h>
#include <rogue/ItemEffect.h>

namespace rogue {

// Apply buff effects
using PoisonDebuffEffect =
    ApplyBuffItemEffect<PoisonDebuffComp, true, HealthComp>;

using BleedingDebuffEffect =
    ApplyBuffItemEffect<BleedingDebuffComp, true, HealthComp>;

using StatsBuffEffect = ApplyBuffItemEffect<StatsBuffComp, false, StatsComp>;

using ArmorBuffEffect = ApplyBuffItemEffect<ArmorBuffComp, false, HealthComp>;

using BlockBuffEffect = ApplyBuffItemEffect<BlockBuffComp, false, HealthComp>;

using HealthRegenBuffEffect =
    ApplyBuffItemEffect<HealthRegenBuffComp, false, HealthComp>;

using ManaRegenBuffEffect =
    ApplyBuffItemEffect<ManaRegenBuffComp, false, ManaComp>;

using BlindedDebuffEffect =
    ApplyBuffItemEffect<BlindedDebuffComp, true, LineOfSightComp>;

using MindVisionBuffEffect = ApplyBuffItemEffect<MindVisionBuffComp, false>;

using InvisibilityBuffEffect = ApplyBuffItemEffect<InvisibilityBuffComp, false>;

using StatsBuffPerHitEffect =
    ApplyBuffItemEffect<StatsBuffPerHitComp, false, StatsComp>;

using CoHTargetBleedingDebuffEffect =
    ApplyBuffItemEffect<CoHTargetBleedingDebuffComp, false>;

using CoHTargetPoisonDebuffEffect =
    ApplyBuffItemEffect<CoHTargetPoisonDebuffComp, false>;

using CoHTargetBlindedDebuffEffect =
    ApplyBuffItemEffect<CoHTargetBlindedDebuffComp, false>;

// Set component effects

class SetMeleeCompEffect : public SetComponentEffect<MeleeAttackComp, false> {
public:
  using SetComponentEffect<MeleeAttackComp, false>::SetComponentEffect;
  std::shared_ptr<ItemEffect> clone() const final;
  std::string getName() const final;
  std::string getDescription() const final;
};

class SetRangedCompEffect : public SetComponentEffect<RangedAttackComp, false> {
public:
  using SetComponentEffect<RangedAttackComp, false>::SetComponentEffect;
  std::shared_ptr<ItemEffect> clone() const final;
  std::string getName() const final;
  std::string getDescription() const final;
};

// Removal effects

class RemovePoisonEffect : public RemoveEffect<PoisonDebuffEffect> {
public:
  using RemoveEffect<PoisonDebuffEffect>::RemoveEffect;
  std::shared_ptr<ItemEffect> clone() const final;
  std::string getName() const final;
  std::string getDescription() const final;
};

// Remove buff effects

class RemovePoisonDebuffEffect
    : public RemoveComponentEffect<PoisonDebuffComp, false> {
public:
  using RemoveComponentEffect<PoisonDebuffComp, false>::RemoveComponentEffect;
  std::shared_ptr<ItemEffect> clone() const final;
  std::string getName() const final;
  std::string getDescription() const final;
};

// Combat

class ManaItemEffect : public ItemEffect {
public:
  explicit ManaItemEffect(StatValue Amount);
  std::shared_ptr<ItemEffect> clone() const final;
  std::string getName() const final;
  std::string getDescription() const final;
  bool canApplyTo(const entt::entity &Et, const entt::entity &DstEt,
                  entt::registry &Reg) const final;
  void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
               entt::registry &Reg) const final;

private:
  StatValue Amount;
};

class SweepingStrikeEffect : public ItemEffect {
public:
  std::shared_ptr<ItemEffect> clone() const final;
  std::string getName() const final;
  std::string getDescription() const final;
  bool canApplyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg) const final;
  void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
               entt::registry &Reg) const final;
};

// Knowledge

class LearnRecipeEffect : public ItemEffect {
public:
  std::shared_ptr<ItemEffect> clone() const final;
  std::string getName() const final;
  std::string getDescription() const final;
  bool canApplyTo(const entt::entity &Et, const entt::entity &DstEt,
                  entt::registry &Reg) const final;
  void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
               entt::registry &Reg) const final;
};

} // namespace rogue

#endif // ROGUE_ITEM_EFFECT_IMPL_H
