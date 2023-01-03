#include "Systems/DeathSystem.h"
#include "Components/Stats.h"
#include "Components/Player.h"
#include <entt/entt.hpp>

void DeathSystem::update() {
  auto View = Reg.view<const HealthComp>();
  View.each([this](const auto &Entity, const auto &H) {
    if (H.Health != 0) {
      return;
    }
    bool IsPlayer = Reg.any_of<PlayerComp>(Entity);
    publish(EntityDiedEvent{{}, Entity, IsPlayer});
    Reg.destroy(Entity);
  });
}