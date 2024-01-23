#include <random>
#include <rogue/Components/AI.h>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/RaceFaction.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/ItemEffect.h>
#include <rogue/Systems/AttackAISystem.h>

namespace rogue {

namespace {

std::random_device RD;

void handleAutoAttack(entt::registry &Reg, entt::entity Entity,
                      const PositionComp &Pos, AgilityComp &Ag,
                      const FactionComp &Fac, entt::entity TEntity,
                      const PositionComp &TPos, const HealthComp &THealth,
                      const FactionComp &TFac) {
  if (THealth.isDead()) {
    return;
  }
  if (Fac.Faction == TFac.Faction) {
    return; // same faction
  }
  auto APCost = MovementComp::MoveAPCost;
  if (auto *MA = Reg.try_get<MeleeAttackComp>(Entity)) {
    APCost = MA->APCost;
  }

  auto Dist =
      std::abs(Pos.Pos.X - TPos.Pos.X) + std::abs(Pos.Pos.Y - TPos.Pos.Y);
  if (Dist <= 1 && Ag.hasEnoughAP(APCost)) {
    auto &CAC = Reg.get_or_emplace<CombatActionComp>(Entity);
    CAC.Target = TEntity;
    CAC.RangedPos = std::nullopt;
  }
}

void updateEffectExecutor(entt::registry &Reg, entt::entity Entity,
                          EffectExecutorComp &Executer) {
  if (Executer.DelayLeft > 0) {
    Executer.DelayLeft--;
    return;
  }

  auto &Effect = Executer.Effects.at(Executer.NextEffect);
  if (Effect.Effect->canApplyTo(Entity, Entity, Reg)) {
    Effect.Effect->applyTo(Entity, Entity, Reg);
  }

  Executer.DelayLeft = Effect.DelayDist(RD);
  Executer.NextEffect++;
  if (Executer.NextEffect >= Executer.Effects.size()) {
    Executer.NextEffect = 0;
  }
}

} // namespace

void AttackAISystem::update(UpdateType Type) {
  if (Type == UpdateType::NoTick) {
    return;
  }

  auto View = Reg.view<const PositionComp, AttackAIComp, AgilityComp,
                       const FactionComp>();
  auto TargetView =
      Reg.view<const PositionComp, HealthComp, const FactionComp>();
  View.each([&TargetView, this](const auto &Entity, const auto &Pos, auto &Ag,
                                const auto &Fac) {
    // FIXME could use lookup map
    TargetView.each([&Entity, &Pos, &Fac, &Ag,
                     this](const auto &TEntity, const auto &TPos, auto &THealth,
                           const auto &TFac) {
      handleAutoAttack(Reg, Entity, Pos, Ag, Fac, TEntity, TPos, THealth, TFac);
    });
  });

  Reg.view<EffectExecutorComp>().each([&](auto Entity, auto &Executer) {
    updateEffectExecutor(Reg, Entity, Executer);
  });
}

} // namespace rogue