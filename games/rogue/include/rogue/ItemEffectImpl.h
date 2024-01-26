#ifndef ROGUE_ITEM_EFFECT_IMPL_H
#define ROGUE_ITEM_EFFECT_IMPL_H

#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Stats.h>
#include <rogue/ItemEffect.h>
#include <rogue/Tile.h>

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

using StatsTimedBuffEffect =
    ApplyBuffItemEffect<StatsTimedBuffComp, false, StatsComp>;

using StatsBuffPerHitEffect =
    ApplyBuffItemEffect<StatsBuffPerHitComp, false, StatsComp>;

using CoHTargetBleedingDebuffEffect =
    ApplyBuffItemEffect<CoHTargetBleedingDebuffComp, false>;

using CoHTargetPoisonDebuffEffect =
    ApplyBuffItemEffect<CoHTargetPoisonDebuffComp, false>;

using CoHTargetBlindedDebuffEffect =
    ApplyBuffItemEffect<CoHTargetBlindedDebuffComp, false>;

using LifeStealBuffEffect =
    ApplyBuffItemEffect<LifeStealBuffComp, false, HealthComp>;

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

#define ROGUE_REMOVE_EFFECT(NAME, COMP)                                        \
  class NAME : public RemoveEffect<COMP> {                                     \
  public:                                                                      \
    using RemoveEffect<COMP>::RemoveEffect;                                    \
    std::shared_ptr<ItemEffect> clone() const final;                           \
    std::string getName() const final;                                         \
    std::string getDescription() const final;                                  \
  };

ROGUE_REMOVE_EFFECT(RemovePoisonEffect, PoisonDebuffComp)
ROGUE_REMOVE_EFFECT(RemoveBleedingEffect, BleedingDebuffComp)
ROGUE_REMOVE_EFFECT(RemoveBlindedEffect, BlindedDebuffComp)
ROGUE_REMOVE_EFFECT(RemoveHealthRegenEffect, HealthRegenBuffComp)
ROGUE_REMOVE_EFFECT(RemoveManaRegenEffect, ManaRegenBuffComp)

// Remove buff effects

#define ROGUE_REMOVE_BUFF_EFFECT(NAME, BUFF, COMBAT)                           \
  class NAME : public RemoveComponentEffect<BUFF, COMBAT> {                    \
  public:                                                                      \
    using RemoveComponentEffect<BUFF, COMBAT>::RemoveComponentEffect;          \
    std::shared_ptr<ItemEffect> clone() const final;                           \
    std::string getName() const final;                                         \
    std::string getDescription() const final;                                  \
  };

ROGUE_REMOVE_BUFF_EFFECT(RemovePoisonDebuffEffect, PoisonDebuffComp, false)
ROGUE_REMOVE_BUFF_EFFECT(RemoveBleedingDebuffEffect, BleedingDebuffComp, false)
ROGUE_REMOVE_BUFF_EFFECT(RemoveBlindedDebuffEffect, BlindedDebuffComp, false)
ROGUE_REMOVE_BUFF_EFFECT(RemoveHealthRegenBuffEffect, HealthRegenBuffComp, true)
ROGUE_REMOVE_BUFF_EFFECT(RemoveManaRegenBuffEffect, ManaRegenBuffComp, true)

#undef ROGUE_REMOVE_BUFF_EFFECT

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
  SweepingStrikeEffect(std::string Name, double DamagePercent, Tile EffectTile);

  std::shared_ptr<ItemEffect> clone() const final;
  std::string getName() const final;
  std::string getDescription() const final;
  bool canApplyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg) const final;
  void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
               entt::registry &Reg) const final;

private:
  std::string Name;
  double DamagePercent;
  Tile EffectTile;
};

class SmiteEffect : public ItemEffect {
public:
  /// Smite effect deals melee damage scaled with the given percent
  explicit SmiteEffect(std::string Name, double DamagePercent);

  std::shared_ptr<ItemEffect> clone() const final;
  std::string getName() const final;
  std::string getDescription() const final;
  bool canApplyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg) const final;
  void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
               entt::registry &Reg) const final;

private:
  std::string Name;
  double DamagePercent;
};

class DiscAreaHitEffect : public ItemEffect {
public:
  DiscAreaHitEffect(std::string Name, unsigned Radius, StatValue PhysDamage,
                    StatValue MagicDamage,
                    std::optional<CoHTargetBleedingDebuffComp> BleedingDebuff,
                    std::optional<CoHTargetPoisonDebuffComp> PoisonDebuff,
                    std::optional<CoHTargetBlindedDebuffComp> BlindedDebuff,
                    Tile EffectTile, double DecreasePercent, unsigned MinTicks,
                    unsigned MaxTicks, bool CanHurtSource, bool CanHurtFaction);

  std::shared_ptr<ItemEffect> clone() const final;
  std::string getName() const final;
  std::string getDescription() const final;
  bool canApplyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg) const final;
  void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
               entt::registry &Reg) const final;

protected:
  void createDamageEt(entt::registry &Reg, const entt::entity &SrcEt,
                      ymir::Point2d<int> Pos, double DecreaseFactor) const;

private:
  std::string Name;
  unsigned Radius;
  StatValue PhysDamage;
  StatValue MagicDamage;

  std::optional<CoHTargetBleedingDebuffComp> BleedingDebuff;
  std::optional<CoHTargetPoisonDebuffComp> PoisonDebuff;
  std::optional<CoHTargetBlindedDebuffComp> BlindedDebuff;

  Tile EffectTile;
  double DecreasePercent;
  unsigned MinTicks;
  unsigned MaxTicks;
  bool CanHurtSource;
  bool CanHurtFaction;
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

// TODO create new effect similar to stomp effect creating damage entities
// in an area, this should allow to add chance on hit effects etc.
class SpawnEntityEffect : public ItemEffect {
public:
  SpawnEntityEffect(std::string EntityName, double Chance);
  std::shared_ptr<ItemEffect> clone() const final;
  std::string getName() const final;
  std::string getDescription() const final;
  bool canApplyTo(const entt::entity &Et, const entt::entity &DstEt,
                  entt::registry &Reg) const final;
  void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
               entt::registry &Reg) const final;

private:
  std::string EntityName;
  double Chance;
};

} // namespace rogue

#endif // ROGUE_ITEM_EFFECT_IMPL_H
