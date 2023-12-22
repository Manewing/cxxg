#ifndef ROGUE_ITEM_EFFECT_IMPL_H
#define ROGUE_ITEM_EFFECT_IMPL_H

#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Stats.h>
#include <rogue/ItemEffect.h>

namespace rogue {

// Apply buff effects
using PoisonDebuffEffect = ApplyBuffItemEffect<PoisonDebuffComp, HealthComp>;

using BleedingDebuffEffect =
    ApplyBuffItemEffect<BleedingDebuffComp, HealthComp>;

using StatsBuffEffect = ApplyBuffItemEffect<StatsBuffComp, StatsComp>;

using ArmorBuffEffect = ApplyBuffItemEffect<ArmorBuffComp, HealthComp>;

using BlockBuffEffect = ApplyBuffItemEffect<BlockBuffComp, HealthComp>;

using HealthRegenBuffEffect =
    ApplyBuffItemEffect<HealthRegenBuffComp, HealthComp>;

using BlindedDebuffEffect =
    ApplyBuffItemEffect<BlindedDebuffComp, LineOfSightComp>;

using MindVisionBuffEffect = ApplyBuffItemEffect<MindVisionBuffComp>;

using InvisibilityBuffEffect = ApplyBuffItemEffect<InvisibilityBuffComp>;

using StatsBuffPerHitEffect =
    ApplyBuffItemEffect<StatsBuffPerHitComp, StatsComp>;

using CoHTargetBleedingDebuffEffect =
    ApplyBuffItemEffect<CoHTargetBleedingDebuffComp>;

using CoHTargetPoisonDebuffEffect =
    ApplyBuffItemEffect<CoHTargetPoisonDebuffComp>;

using CoHTargetBlindedDebuffEffect =
    ApplyBuffItemEffect<CoHTargetBlindedDebuffComp>;

// Set component effects

class SetMeleeCompEffect : public SetComponentEffect<MeleeAttackComp> {
public:
  using SetComponentEffect<MeleeAttackComp>::SetComponentEffect;
  std::shared_ptr<ItemEffect> clone() const final;
  std::string getName() const final;
  std::string getDescription() const final;
};

class SetRangedCompEffect : public SetComponentEffect<RangedAttackComp> {
public:
  using SetComponentEffect<RangedAttackComp>::SetComponentEffect;
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
    : public RemoveComponentEffect<PoisonDebuffComp> {
public:
  using RemoveComponentEffect<PoisonDebuffComp>::RemoveComponentEffect;
  std::shared_ptr<ItemEffect> clone() const final;
  std::string getName() const final;
  std::string getDescription() const final;
};

// Combat

class SweepingStrikeEffect : public ItemEffect {
public:
  std::shared_ptr<ItemEffect> clone() const final;
  std::string getName() const final;
  std::string getDescription() const final;
  bool canApplyTo(const entt::entity &Et, entt::registry &Reg) const final;
  void applyTo(const entt::entity &Et, entt::registry &Reg) const final;
};

} // namespace rogue

#endif // ROGUE_ITEM_EFFECT_IMPL_H
