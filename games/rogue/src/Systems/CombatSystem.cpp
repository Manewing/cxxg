#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Visual.h>
#include <rogue/Event.h>
#include <rogue/History.h>
#include <rogue/Systems/CombatSystem.h>

namespace rogue {

namespace {

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

  // Compute the effective damage and apply it
  auto *SC = Reg.try_get<StatsComp>(Attacker);
  unsigned Damage = MA->getEffectiveDamage(SC);
  if (auto *ABC = Reg.try_get<ArmorBuffComp>(Target)) {
    Damage = ABC->getEffectiveDamage(Damage, Reg.try_get<StatsComp>(Target));
  }
  THealth->reduce(Damage);

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
}

} // namespace rogue