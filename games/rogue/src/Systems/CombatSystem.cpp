#include <random>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Event.h>
#include <rogue/History.h>
#include <rogue/Systems/CombatSystem.h>

namespace rogue {

namespace {

// FIXME move this, also should this be based on the level seed?
static std::random_device RandomEngine;

/// Applies combat components to entities either being the target of an attack
/// or the attacker.
/// Always keep the last attacker and target in the combat component
void applyCombatComps(entt::registry &Reg, entt::entity Attacker,
                      entt::entity Target) {
  Reg.get_or_emplace<CombatAttackComp>(Attacker).Target = Target;
  if (Target != entt::null) {
    Reg.get_or_emplace<CombatTargetComp>(Target).Attacker = Attacker;
  }
}

template <typename ChanceOnHitBuffType>
void tryApplyChanceOnHitBuff(entt::registry &Reg, entt::entity Target,
                             entt::entity Source) {
  if (auto *COHB = Reg.try_get<ChanceOnHitBuffType>(Source)) {
    if (COHB->canApplyTo(Source, Target, Reg)) {
      COHB->applyTo(Source, Target, Reg);
    }
  }
}

std::optional<unsigned> applyDamage(entt::registry &Reg,
                                    const entt::entity Target,
                                    HealthComp &THealth, const DamageComp &DC) {
  if (auto *BC = Reg.try_get<BlockBuffComp>(Target)) {
    // Increase the block chance if the attacker is blinded
    StatValue MaxChance =
        Reg.any_of<BlindedDebuffComp>(DC.Source) ? 100.0 : 50.0;

    std::uniform_real_distribution<StatValue> Chance(0, MaxChance);
    if (Chance(RandomEngine) <= BC->BlockChance) {
      return std::nullopt;
    }
  }

  DamageComp NewDC = DC;
  if (auto *ABC = Reg.try_get<ArmorBuffComp>(Target)) {
    auto *SC = Reg.try_get<StatsComp>(Target);
    NewDC.PhysDamage = ABC->getPhysEffectiveDamage(NewDC.PhysDamage, SC);
    NewDC.PhysDamage = ABC->getMagicEffectiveDamage(NewDC.PhysDamage, SC);
  }

  tryApplyChanceOnHitBuff<CoHTargetBleedingDebuffComp>(Reg, Target, DC.Source);
  tryApplyChanceOnHitBuff<CoHTargetBlindedDebuffComp>(Reg, Target, DC.Source);
  tryApplyChanceOnHitBuff<CoHTargetPoisonDebuffComp>(Reg, Target, DC.Source);

  THealth.reduce(NewDC.PhysDamage);
  THealth.reduce(NewDC.MagicDamage);
  return static_cast<unsigned>(NewDC.total());
}

bool performMeleeAttack(entt::registry &Reg, entt::entity Attacker,
                        entt::entity Target, EventHubConnector &EHC) {
  auto *THealth = Reg.try_get<HealthComp>(Target);
  if (!THealth) {
    return true;
  }

  // Melee is always possible, used default values for damage if not set
  const MeleeAttackComp DefaultMA = {/*PhysDamage = */ 1, /*MagicDamage = */ 0,
                                     /*APCost = */ 10};
  MeleeAttackComp MA = DefaultMA;
  auto *AMA = Reg.try_get<MeleeAttackComp>(Attacker);
  if (AMA) {
    MA = *AMA;
  }

  // Check if the attacker has enough AP
  auto *AG = Reg.try_get<AgilityComp>(Attacker);
  if (AG && !AG->trySpendAP(MA.APCost)) {
    return false;
  }

  // If there is a mana cost, check if the attacker has enough mana. This if not
  // remove the combat component, we still consume AP for trying the action
  if (MA.ManaCost > 0) {
    auto *MC = Reg.try_get<ManaComp>(Attacker);
    if (!MC || !MC->tryReduce(MA.ManaCost)) {
      if (Reg.any_of<PlayerComp>(Attacker)) {
        EHC.publish(PlayerInfoMessageEvent() << "Not enough mana");
      }
      return true;
    }
  }

  CombatSystem::handleMeleeAttack(Reg, Attacker, Target, EHC);

  return true;
}

/// Performs a range attack by creating a projectile based on the computed
/// damage for the attacker
/// \return true if the combat component can be removed
bool performRangedAttack(entt::registry &Reg, entt::entity Attacker,
                         ymir::Point2d<int> TargetPos, EventHubConnector &EHC) {
  auto *AttackerPC = Reg.try_get<PositionComp>(Attacker);
  if (!AttackerPC) {
    return true;
  }

  auto *RAC = Reg.try_get<RangedAttackComp>(Attacker);
  if (!RAC) {
    return true;
  }

  // Check if the attacker has enough AP
  auto *AG = Reg.try_get<AgilityComp>(Attacker);
  if (AG && !AG->trySpendAP(RAC->APCost)) {
    return false;
  }

  // If there is a mana cost, check if the attacker has enough mana. This if not
  // remove the combat component, we still consume AP for trying the action
  if (RAC->ManaCost > 0) {
    auto *MC = Reg.try_get<ManaComp>(Attacker);
    if (!MC || !MC->tryReduce(RAC->ManaCost)) {
      if (Reg.any_of<PlayerComp>(Attacker)) {
        EHC.publish(PlayerInfoMessageEvent() << "Not enough mana");
      }
      return true;
    }
  }

  // Compute the effective damage and apply it
  DamageComp DC;
  DC.Source = Attacker;
  if (auto *SC = Reg.try_get<StatsComp>(Attacker)) {
    auto SP = SC->effective();
    DC.PhysDamage = RAC->getPhysEffectiveDamage(&SP);
    DC.MagicDamage = RAC->getMagicEffectiveDamage(&SP);
  } else {
    DC.PhysDamage = RAC->getPhysEffectiveDamage();
    DC.MagicDamage = RAC->getMagicEffectiveDamage();
  }

  // Create projectile in the move direction
  auto Diff = TargetPos - AttackerPC->Pos;
  auto MoveDir = ymir::Dir2d::fromVector(Diff);
  auto PPos = AttackerPC->Pos + MoveDir;
  createProjectile(Reg, DC, PPos, TargetPos, 100);

  return true;
}

void performAttack(entt::registry &Reg, entt::entity Attacker,
                   const CombatActionComp &CC, EventHubConnector &EHC) {
  assert(CC.Target != entt::null || CC.RangedPos.has_value());
  bool CanRemove = false;
  if (CC.RangedPos) {
    CanRemove = performRangedAttack(Reg, Attacker, *CC.RangedPos, EHC);
  } else {
    CanRemove = performMeleeAttack(Reg, Attacker, CC.Target, EHC);
  }
  if (CanRemove) {
    Reg.erase<CombatActionComp>(Attacker);
    applyCombatComps(Reg, Attacker, CC.Target);
  }
}

void applyDamageComp(entt::registry &Reg, DamageComp &DC,
                 const PositionComp &PC, EventHubConnector &EHC) {
  Reg.view<PositionComp, HealthComp>().each(
      [&Reg, &PC, &DC, &EHC](const auto &TEt, auto &TPC, auto &THC) {
        auto *TMC = Reg.try_get<MovementComp>(TEt);
        if (PC.Pos != TPC.Pos && (!TMC || (TPC.Pos + TMC->Dir) != PC.Pos)) {
          return;
        }
        if (DC.Hits-- == 0) {
          return;
        }
        auto Damage = applyDamage(Reg, TEt, THC, DC);

        // Applying damage may cause update attack/target components
        applyCombatComps(Reg, DC.Source, TEt);

        // publish
        EntityAttackEvent EAE;
        EAE.Registry = &Reg;
        EAE.Attacker = DC.Source;
        EAE.Target = TEt;
        EAE.Damage = Damage;
        EHC.publish(EAE);
      });
}

} // namespace

/// Returns true if the combat component can be removed
void CombatSystem::handleMeleeAttack(entt::registry &Reg, entt::entity Attacker,
                                     entt::entity Target,
                                     EventHubConnector &EHC,
                                     float DamageFactor) {
  // Melee is always possible, used default values for damage if not set
  const MeleeAttackComp DefaultMA = {/*PhysDamage = */ 1, /*MagicDamage = */ 0,
                                     /*APCost = */ 10};
  MeleeAttackComp MA = DefaultMA;
  auto *AMA = Reg.try_get<MeleeAttackComp>(Attacker);
  if (AMA) {
    MA = *AMA;
  }

  auto *THealth = Reg.try_get<HealthComp>(Target);
  if (!THealth) {
    return;
  }

  // Compute the effective damage and apply it
  DamageComp DC;
  DC.Source = Attacker;
  if (auto *SC = Reg.try_get<StatsComp>(Attacker)) {
    auto SP = SC->effective();
    DC.PhysDamage = MA.getPhysEffectiveDamage(&SP);
    DC.MagicDamage = MA.getMagicEffectiveDamage(&SP);
  } else {
    DC.PhysDamage = MA.getPhysEffectiveDamage();
    DC.MagicDamage = MA.getMagicEffectiveDamage();
  }
  DC.PhysDamage *= DamageFactor;
  DC.MagicDamage *= DamageFactor;
  auto TotalDamage = applyDamage(Reg, Target, *THealth, DC);

  // Check for on hit buffs and apply stacks
  if (auto *SBPH = Reg.try_get<StatsBuffPerHitComp>(Attacker);
      TotalDamage && SBPH) {
    SBPH->addStack();
  }

  // Applying damage may cause update attack/target components
  applyCombatComps(Reg, Attacker, Target);

  // publish
  EntityAttackEvent EAE;
  EAE.Registry = &Reg;
  EAE.Attacker = Attacker;
  EAE.Target = Target;
  EAE.Damage = TotalDamage;
  EHC.publish(EAE);
}

void CombatSystem::update(UpdateType Type) {
  if (Type == UpdateType::NoTick) {
    return;
  }

  // Deal with melee attacks
  Reg.view<CombatActionComp>().each([this](const auto &AttackerEt, auto &CC) {
    performAttack(Reg, AttackerEt, CC, *this);
  });

  // Deal with damages, removing damage entities is handled by the death system
  Reg.view<DamageComp, PositionComp>().each(
      [this](const auto &, auto &DC, auto &PC) {
        applyDamageComp(Reg, DC, PC, *this);
      });
}

} // namespace rogue