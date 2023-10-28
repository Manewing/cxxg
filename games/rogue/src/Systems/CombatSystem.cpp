#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Event.h>
#include <rogue/History.h>
#include <rogue/Systems/CombatSystem.h>

namespace rogue {

namespace {

unsigned applyDamage(entt::registry &Reg, const entt::entity Target,
                     HealthComp &THealth, const DamageComp &DC) {
  DamageComp NewDC = DC;
  if (auto *ABC = Reg.try_get<ArmorBuffComp>(Target)) {
    NewDC.PhysDamage = ABC->getEffectiveDamage(NewDC.PhysDamage,
                                               Reg.try_get<StatsComp>(Target));
  }
  THealth.reduce(NewDC.PhysDamage);
  THealth.reduce(NewDC.MagicDamage);
  return NewDC.total();
}

/// Returns true if the combat component can be removed
bool performMeleeAttack(entt::registry &Reg, entt::entity Attacker,
                        entt::entity Target, EventHubConnector &EHC) {
  auto *THealth = Reg.try_get<HealthComp>(Target);
  if (!THealth) {
    return true;
  }

  // Melee is always possible, used default values for damage if not set
  const MeleeAttackComp DefaultMA = {
      .PhysDamage = 1, .MagicDamage = 0, .APCost = 10};
  MeleeAttackComp MA = DefaultMA;
  auto *AMA = Reg.try_get<MeleeAttackComp>(Attacker);
  if (AMA) {
    MA = *AMA;
  }

  auto *AG = Reg.try_get<AgilityComp>(Attacker);
  if (AG && !AG->trySpendAP(MA.APCost)) {
    return false;
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
  unsigned TotalDamage = applyDamage(Reg, Target, *THealth, DC);

  // Check for on hit buffs and apply stacks
  if (auto *SBPH = Reg.try_get<StatsBuffPerHitComp>(Attacker)) {
    SBPH->addStack();
  }

  // publish
  EntityAttackEvent EAE;
  EAE.Registry = &Reg;
  EAE.Attacker = Attacker;
  EAE.Target = Target;
  EAE.Damage = TotalDamage;
  EHC.publish(EAE);

  return true;
}

/// Returns true if the combat component can be removed
bool performRangedAttack(entt::registry &Reg, entt::entity Attacker,
                         ymir::Point2d<int> TargetPos) {
  auto *AttackerPC = Reg.try_get<PositionComp>(Attacker);
  if (!AttackerPC) {
    return true;
  }

  auto *RAC = Reg.try_get<RangedAttackComp>(Attacker);
  if (!RAC) {
    return true;
  }

  auto *AG = Reg.try_get<AgilityComp>(Attacker);
  if (AG && !AG->trySpendAP(RAC->APCost)) {
    return false;
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
                   const CombatComp &CC, EventHubConnector &EHC) {
  assert(CC.Target != entt::null || CC.RangedPos.has_value());
  bool CanRemove = false;
  if (CC.RangedPos) {
    CanRemove = performRangedAttack(Reg, Attacker, *CC.RangedPos);
  } else {
    CanRemove = performMeleeAttack(Reg, Attacker, CC.Target, EHC);
  }
  if (CanRemove) {
    Reg.erase<CombatComp>(Attacker);
  }
}

void applyDamage(entt::registry &Reg, entt::entity DmgEt, DamageComp &DC,
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

        // publish
        EntityAttackEvent EAE;
        EAE.Registry = &Reg;
        EAE.Attacker = DC.Source;
        EAE.Target = TEt;
        EAE.Damage = Damage;
        EHC.publish(EAE);
      });

  if (DC.Hits <= 0) {
    Reg.destroy(DmgEt);
  }
}

} // namespace

void CombatSystem::update(UpdateType Type) {
  if (Type == UpdateType::NoTick) {
    return;
  }

  // Deal with melee attacks
  Reg.view<CombatComp>().each([this](const auto &AttackerEt, auto &CC) {
    performAttack(Reg, AttackerEt, CC, *this);
  });

  // Deal with damages
  Reg.view<DamageComp, PositionComp>().each(
      [this](const auto &DmgEt, auto &DC, auto &PC) {
        applyDamage(Reg, DmgEt, DC, PC, *this);
      });

}

} // namespace rogue