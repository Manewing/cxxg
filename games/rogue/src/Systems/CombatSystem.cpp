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
                     HealthComp &THealth, unsigned Damage) {
  if (auto *ABC = Reg.try_get<ArmorBuffComp>(Target)) {
    Damage = ABC->getEffectiveDamage(Damage, Reg.try_get<StatsComp>(Target));
  }
  THealth.reduce(Damage);
  return Damage;
}

void performMeleeAttack(entt::registry &Reg, entt::entity Attacker,
                        entt::entity Target, EventHubConnector &EHC) {
  auto *THealth = Reg.try_get<HealthComp>(Target);
  if (!THealth) {
    return;
  }

  auto *MA = Reg.try_get<MeleeAttackComp>(Attacker);
  if (!MA) {
    return;
  }

  auto *AG = Reg.try_get<AgilityComp>(Attacker);
  if (AG && !AG->trySpendAP(MA->APCost)) {
    return;
  }

  // Compute the effective damage and apply it
  auto *SC = Reg.try_get<StatsComp>(Attacker);
  unsigned Damage = MA->getEffectiveDamage(SC);
  Damage = applyDamage(Reg, Target, *THealth, Damage);

  // Check for on hit buffs and apply stacks
  if (auto *SBPH = Reg.try_get<StatsBuffPerHitComp>(Attacker)) {
    SBPH->addStack();
  }

  // publish
  EntityAttackEvent EAE;
  EAE.Registry = &Reg;
  EAE.Attacker = Attacker;
  EAE.Target = Target;
  EAE.Damage = Damage;
  EHC.publish(EAE);
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
        auto Damage = applyDamage(Reg, TEt, THC, DC.Damage);

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
    performMeleeAttack(Reg, AttackerEt, CC.Target, *this);
    Reg.erase<CombatComp>(AttackerEt);
  });

  // Deal with damages
  Reg.view<DamageComp, PositionComp>().each(
      [this](const auto &DmgEt, auto &DC, auto &PC) {
        applyDamage(Reg, DmgEt, DC, PC, *this);
      });
}

} // namespace rogue