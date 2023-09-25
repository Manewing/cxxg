#include <entt/entt.hpp>
#include <rogue/Components/Entity.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Systems/DeathSystem.h>

namespace rogue {

namespace {

std::optional<Inventory>
getDropInventoryFromEntity(entt::entity Entity, entt::registry &Reg){
  std::optional<Inventory> Inv;
  if (auto *IC = Reg.try_get<InventoryComp>(Entity)) {
    Inv = IC->Inv;
  }
  if (auto *EC = Reg.try_get<EquipmentComp>(Entity)) {
    if (!Inv) {
      Inv = Inventory{};
    }
    for (const auto &ES : EC->Equip.all()) {
      if (!ES->empty()) {
        Inv->addItem(ES->unequip());
      }
    }
  }
  return Inv;
} 

}

void DeathSystem::update(UpdateType Type) {
  (void)Type; // Always run

  auto View = Reg.view<const HealthComp, const PositionComp>();
  View.each([this](const auto &Entity, const auto &Health, const auto &Pos) {
    if (Health.Value != 0) {
      return;
    }
    bool IsPlayer = Reg.any_of<PlayerComp>(Entity);
    publish(EntityDiedEvent{{}, Entity, IsPlayer});

    // Create loot from entity
    if (auto InvOrNone = getDropInventoryFromEntity(Entity, Reg)) {
      createDropEntity(Reg, Pos.Pos, InvOrNone.value());
    }

    // FIXME player can't die at the moment
    if (!IsPlayer) {
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