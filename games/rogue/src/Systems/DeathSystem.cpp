#include "Systems/DeathSystem.h"
#include "Components/Player.h"
#include "Components/Stats.h"
#include "Components/Transform.h"
#include "Components/Entity.h"
#include "Components/Items.h"
#include <entt/entt.hpp>

void DeathSystem::update() {
  auto View = Reg.view<const HealthComp, const PositionComp>();
  View.each([this](const auto &Entity, const auto &Health, const auto &Pos) {
    if (Health.Value != 0) {
      return;
    }
    bool IsPlayer = Reg.any_of<PlayerComp>(Entity);
    publish(EntityDiedEvent{{}, Entity, IsPlayer});

    // FIXME player can't die at the moment
    if (!IsPlayer) {
      // FIXME create loot generation system
      createDropEntity(Reg, Pos);

      Reg.destroy(Entity);
    }
  });

  // FIXME should this be done somewhere else?
  auto DropView = Reg.view<const DropComp>();
  DropView.each([this](const auto &Entity, const auto &Drop) {
    if (Drop.Inv.empty()) {
      Reg.destroy(Entity);
    }
  });
}