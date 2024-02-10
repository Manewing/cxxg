#include <entt/entt.hpp>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Entity.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Context.h>
#include <rogue/Event.h>
#include <rogue/ItemDatabase.h>
#include <rogue/Systems/DeathSystem.h>

namespace rogue {

namespace {

Inventory getDropInventoryFromEntity(entt::entity Entity, entt::registry &Reg) {
  Inventory Inv;

  if (auto *IC = Reg.try_get<InventoryComp>(Entity)) {
    Inv = IC->Inv;
  }

  if (Reg.any_of<DropEquipmentComp>(Entity)) {
    if (auto *EC = Reg.try_get<EquipmentComp>(Entity)) {
      for (const auto &ES : EC->Equip.all()) {
        if (!ES->empty()) {
          Inv.addItem(ES->unequip());
        }
      }
    }
  }

  return Inv;
}

void reapEntity(entt::registry &Reg, EventHubConnector &EHC,
                const entt::entity Entity) {
  EHC.publish(EntityDiedEvent{{}, Entity, &Reg});

  // Create loot from entity
  if (auto *PC = Reg.try_get<PositionComp>(Entity)) {
    if (auto Inv = getDropInventoryFromEntity(Entity, Reg); !Inv.empty()) {
      createDropEntity(Reg, PC->Pos, Inv);
    }
  }

  // Clear combat components, check if target is currently being attacked
  if (auto *CTC = Reg.try_get<CombatTargetComp>(Entity)) {
    if (Reg.valid(CTC->Attacker) &&
        Reg.any_of<CombatAttackComp>(CTC->Attacker)) {
      Reg.erase<CombatAttackComp>(CTC->Attacker);
    }
  }

  Reg.destroy(Entity);
}

void reapDeadEntity(entt::registry &Reg, EventHubConnector &EHC,
                    const entt::entity Entity, const HealthComp &HC) {
  if (HC.Value != 0) {
    return;
  }
  reapEntity(Reg, EHC, Entity);
}

void reapDamageEntity(entt::registry &Reg, EventHubConnector &EHC,
                      const entt::entity Entity, DamageComp &DC,
                      System::UpdateType Type) {
  const auto Ticks = DC.Ticks;
  if (Type == System::UpdateType::Tick) {
    DC.Ticks--;
  }
  if (Ticks > 0 && DC.Hits > 0) {
    return;
  }
  reapEntity(Reg, EHC, Entity);
}

/// Empty inventory will be removed unless there is a health component
void removeEmptyContainers(entt::registry &Reg, const entt::entity Entity,
                           const InventoryComp &IC,
                           const LootInteractComp &LIC) {
  if (Reg.any_of<HealthComp>(Entity)) {
    return;
  }
  if (IC.Inv.empty() && LIC.IsLooted && !LIC.IsPersistent) {
    Reg.destroy(Entity);
  }
}

} // namespace

void DeathSystem::update(UpdateType Type) {
  // Handle dead entities
  auto DeadView = Reg.view<const HealthComp>();
  DeadView.each([this](const auto &Entity, const auto &HC) {
    reapDeadEntity(Reg, *this, Entity, HC);
  });

  // Handle damage components
  auto DmgView = Reg.view<DamageComp>();
  DmgView.each([this, Type](const auto &Entity, auto &DC) {
    reapDamageEntity(Reg, *this, Entity, DC, Type);
  });

  // FIXME should this be done somewhere else?
  auto DropView = Reg.view<const InventoryComp, const LootInteractComp>();
  DropView.each([this](const auto &Entity, const auto &IC, const auto &LIC) {
    removeEmptyContainers(Reg, Entity, IC, LIC);
  });
}

} // namespace rogue