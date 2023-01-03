#include "Systems/AttackAISystem.h"
#include "Components/AI.h"
#include "Components/Stats.h"
#include "Components/Transform.h"
#include "Components/Visual.h"

#include "History.h"

void AttackAISystem::update() {
  auto View = Reg.view<const PositionComp, AttackAIComp, const MeleeAttackComp,
                       AgilityComp, const FactionComp>();
  auto TargetView =
      Reg.view<const PositionComp, HealthComp, const FactionComp>();
  View.each([&TargetView, this](const auto &Entity, const auto &Pos,
                                const auto &MA, auto &Ag, const auto &Fac) {
    // FIXME could use lookup map
    TargetView.each([&Entity, &Pos, &Fac, &MA, &Ag,
                     this](const auto &TEntity, const auto &TPos, auto &THealth,
                           const auto &TFac) {
      if (Fac.Faction == TFac.Faction) {
        return; // same faction
      }
      auto Dist =
          std::abs(Pos.Pos.X - TPos.Pos.X) + std::abs(Pos.Pos.Y - TPos.Pos.Y);
      if (Dist <= 1) {
        if (!Ag.trySpendAP(MA.APCost)) {
          return;
        }

        int NewHealth = THealth.Health - MA.Damage;
        if (NewHealth < 0) {
          NewHealth = 0;
        }

        // publish
        const auto Nm = Reg.try_get<NameComp>(Entity);
        const auto TNm = Reg.try_get<NameComp>(TEntity);
        if (Nm && TNm) {
          publish(DebugMessageEvent() << Nm->Name << " dealt " << MA.Damage
                                      << " melee damage to " << TNm->Name);
        }

        THealth.Health = NewHealth;
      }
    });
  });
}