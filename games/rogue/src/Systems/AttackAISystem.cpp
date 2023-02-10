#include <rogue/Components/AI.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Systems/AttackAISystem.h>

#include <rogue/History.h>

namespace rogue {

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

        THealth.reduce(MA.Damage);

        // publish
        const auto Nm = Reg.try_get<NameComp>(Entity);
        const auto TNm = Reg.try_get<NameComp>(TEntity);
        if (Nm && TNm) {
          publish(DebugMessageEvent() << Nm->Name << " dealt " << MA.Damage
                                      << " melee damage to " << TNm->Name);
        }
      }
    });
  });
}

} // namespace rogue