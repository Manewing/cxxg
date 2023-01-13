#include <entt/entt.hpp>
#include <rogue/Components/Entity.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Systems/DeathSystem.h>

namespace rogue {

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

} // namespace rogue