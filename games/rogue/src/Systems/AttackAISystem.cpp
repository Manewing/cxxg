#include <random>
#include <rogue/Components/AI.h>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/LOS.h>
#include <rogue/Components/RaceFaction.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/ItemEffect.h>
#include <rogue/Level.h>
#include <rogue/Systems/AttackAISystem.h>
#include <ymir/Algorithm/LineOfSight.hpp>

namespace rogue {

namespace {

std::random_device RD;

class EngagedCombat : public std::exception {};

// Checks
void handlePotentialTarget(Level &L, entt::registry &Reg, entt::entity Entity,
                           const PositionComp &Pos, const AgilityComp &Ag,
                           const FactionComp &Fac, entt::entity Target,
                           const PositionComp &TPos, const HealthComp &THealth,
                           const FactionComp &TFac) {
  // We are checking all entities that could be possible targets based on
  // their components, filter out the ones that are not valid targets
  if (THealth.isDead() || Fac.Faction == TFac.Faction) {
    return;
  }

  // We need to distinguish between melee and ranged attacks
  auto APCost = MovementComp::MoveAPCost;
  if (auto *MA = Reg.try_get<MeleeAttackComp>(Entity)) {
    APCost = MA->APCost;
  }

  auto Dist =
      std::abs(Pos.Pos.X - TPos.Pos.X) + std::abs(Pos.Pos.Y - TPos.Pos.Y);
  if (Dist <= 1 && Ag.hasEnoughAP(APCost)) {
    auto &CAC = Reg.get_or_emplace<CombatActionComp>(Entity);
    CAC.Target = Target;
    CAC.RangedPos = std::nullopt;
    throw EngagedCombat();
  }

  auto *RAC = Reg.try_get<RangedAttackComp>(Entity);
  auto *LOS = Reg.try_get<LineOfSightComp>(Entity);
  if (RAC && LOS && static_cast<int>(LOS->LOSRange) >= Dist) {
    auto const Offset = ymir::Point2d<double>(0.5, 0.5);
    if (!ymir::Algorithm::isInLOS<int>(
            [Pos, TPos, &L](auto P) {
              if (P == Pos.Pos || P == TPos.Pos) {
                return false;
              }
              return L.isBodyBlocked(P, /*Hard=*/true);
            },
            Pos.Pos, TPos.Pos, LOS->LOSRange, Offset)) {
      return;
    }

    // If we are far enough away schedule a ranged attack
    static constexpr int SafeDist = 5;
    if (Dist > SafeDist) {
      Reg.emplace<CombatActionComp>(Entity, Target, TPos);
      throw EngagedCombat();
    }
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

void handleRangedAndMeleeAutoAttacks(Level &L) {
  auto View = L.Reg.view<const PositionComp, AttackAIComp, AgilityComp,
                         const FactionComp>();
  auto TargetView =
      L.Reg.view<const PositionComp, HealthComp, const FactionComp>();
  View.each([&TargetView, &L](const auto &Entity, const auto &Pos,
                              const auto &Ag, const auto &Fac) {
    if (L.Reg.any_of<CombatActionComp>(Entity)) {
      if (L.Reg.any_of<MovementComp>(Entity)) {
        L.Reg.erase<MovementComp>(Entity);
      }
      return;
    }
    try {
      TargetView.each(
          [&Entity, &Pos, &Fac, &Ag, &L](const auto &TEntity, const auto &TPos,
                                         auto &THealth, const auto &TFac) {
            handlePotentialTarget(L, L.Reg, Entity, Pos, Ag, Fac, TEntity, TPos,
                                  THealth, TFac);
          });
    } catch (const EngagedCombat &) {
      // We we are in combat we can't move
      if (L.Reg.any_of<MovementComp>(Entity)) {
        L.Reg.erase<MovementComp>(Entity);
      }
    }
  });
}

} // namespace

AttackAISystem::AttackAISystem(Level &L) : System(L.Reg), L(L) {}

void AttackAISystem::update(UpdateType Type) {
  if (Type == UpdateType::NoTick) {
    return;
  }

  handleRangedAndMeleeAutoAttacks(L);

  Reg.view<EffectExecutorComp>().each([&](auto Entity, auto &Executer) {
    updateEffectExecutor(Reg, Entity, Executer);
  });
}

} // namespace rogue