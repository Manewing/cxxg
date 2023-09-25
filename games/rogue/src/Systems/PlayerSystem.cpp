#include <entt/entt.hpp>
#include <rogue/Components/AI.h>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Level.h>
#include <rogue/Systems/PlayerSystem.h>

#include <rogue/History.h>

namespace rogue {

PlayerSystem::PlayerSystem(Level &L) : System(L.Reg), L(L) {}

void PlayerSystem::update(UpdateType Type) {
  if (Type == UpdateType::NoTick) {
    return;
  }

  auto View =
      Reg.view<PlayerComp, PositionComp, const MeleeAttackComp, MovementComp>();
  View.each([this](const auto &PlayerEt, auto &PC, auto &PosComp,
                   const auto &MA, auto &MC) {
    if (MC.Dir == ymir::Dir2d::NONE) {
      return;
    }
    PC.CurrentInteraction = std::nullopt;

    auto NewPos = PosComp.Pos + MC.Dir;
    MC.Dir = ymir::Dir2d::NONE;

    if (auto Et = L.getEntityAt(NewPos);
        Et != entt::null && Reg.all_of<HealthComp, FactionComp>(Et)) {

      // FIXME create damage system and spawn melee damage entity
      auto &THealth = Reg.get<HealthComp>(Et);

      auto *SC = Reg.try_get<StatsComp>(PlayerEt);
      unsigned Damage = MA.getEffectiveDamage(SC);
      if (auto *ABC = Reg.try_get<ArmorBuffComp>(Et)) {
        Damage = ABC->getEffectiveDamage(Damage, Reg.try_get<StatsComp>(Et));
      }
      THealth.reduce(Damage);

      if (auto *SBPH = Reg.try_get<StatsBuffPerHitComp>(PlayerEt)) {
        SBPH->addStack();
      }

      // publish
      const auto Nm = Reg.try_get<NameComp>(PlayerEt);
      const auto TNm = Reg.try_get<NameComp>(Et);
      if (Nm && TNm) {
        publish(DebugMessageEvent() << Nm->Name << " dealt " << Damage
                                    << " melee damage to " << TNm->Name);
      }
      return;
    }

    if (!L.isBodyBlocked(NewPos)) {
      L.updateEntityPosition(PlayerEt, PosComp, NewPos);
    } else {
      publish(DebugMessageEvent() << "Can't move");
    }
  });
}

} // namespace rogue