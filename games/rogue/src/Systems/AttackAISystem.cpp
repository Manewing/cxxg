#include <rogue/Components/AI.h>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/RaceFaction.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Systems/AttackAISystem.h>

namespace rogue {

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
        Reg.emplace<CombatComp>(Entity, TEntity);
      }
    });
  });
}

} // namespace rogue