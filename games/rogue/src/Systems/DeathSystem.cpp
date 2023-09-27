#include <entt/entt.hpp>
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

void addRaceDrops(RaceKind Race, Inventory &Inv, entt::registry &Reg) {
  // FIXME load this from config
  struct DropInfo {
    double DropChance = 0.3;
    int ItemId;

    bool rollDice() const { return (rand() % 100) / 100.0 < DropChance; }
  };
  struct RaceDropInfo {
    std::vector<DropInfo> Drops;
  };
  static std::map<RaceKind, RaceDropInfo> RaceDrops = {
      {RaceKind::Undead, {{DropInfo{0.6, 11}}}},
      {RaceKind::Troll, {{DropInfo{0.1, 12}}}},
  };

  auto It = RaceDrops.find(Race);
  if (It == RaceDrops.end()) {
    return;
  }

  for (const auto &Drop : It->second.Drops) {
    if (Drop.rollDice()) {
      Inv.addItem(
          Reg.ctx().get<GameContext>().ItemDb.createItem(Drop.ItemId, 1));
    }
  }
}

Inventory getDropInventoryFromEntity(entt::entity Entity, entt::registry &Reg) {
  Inventory Inv;

  if (auto *IC = Reg.try_get<InventoryComp>(Entity)) {
    Inv = IC->Inv;
  }
  if (auto *EC = Reg.try_get<EquipmentComp>(Entity)) {
    for (const auto &ES : EC->Equip.all()) {
      if (!ES->empty()) {
        Inv.addItem(ES->unequip());
      }
    }
  }
  if (auto *RC = Reg.try_get<RaceComp>(Entity)) {
    addRaceDrops(RC->Kind, Inv, Reg);
  }
  return Inv;
}

void reapDeadEntities(entt::registry &Reg, EventHubConnector &EHC,
                      const entt::entity Entity, const HealthComp &HC,
                      const PositionComp &PC) {
  if (HC.Value != 0) {
    return;
  }
  EHC.publish(EntityDiedEvent{{}, Entity, &Reg});

  // Create loot from entity
  if (auto Inv = getDropInventoryFromEntity(Entity, Reg); !Inv.empty()) {
    createDropEntity(Reg, PC.Pos, Inv);
  }

  Reg.destroy(Entity);
}

/// Empty inventory will be removed unless there is a health component
void removeEmptyContainers(entt::registry &Reg, const entt::entity Entity,
                           const InventoryComp &IC) {
  if (Reg.any_of<HealthComp>(Entity)) {
    return;
  }
  if (IC.Inv.empty()) {
    Reg.destroy(Entity);
  }
}

} // namespace

void DeathSystem::update(UpdateType Type) {
  (void)Type; // Always run

  auto View = Reg.view<const HealthComp, const PositionComp>();
  View.each([this](const auto &Entity, const auto &HC, const auto &PC) {
    reapDeadEntities(Reg, *this, Entity, HC, PC);
  });

  // FIXME should this be done somewhere else?
  auto DropView = Reg.view<const InventoryComp>();
  DropView.each([this](const auto &Entity, const auto IC) {
    removeEmptyContainers(Reg, Entity, IC);
  });
}

} // namespace rogue