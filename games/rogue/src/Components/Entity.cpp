#include <rogue/Components/AI.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Entity.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/LOS.h>
#include <rogue/Components/Level.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/RaceFaction.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Context.h>
#include <rogue/Event.h>
#include <rogue/EventHub.h>
#include <rogue/InventoryHandler.h>

namespace rogue {

void createEnemy(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                 const std::string &Name, const Inventory &I,
                 const StatPoints &Stats, FactionKind Faction, RaceKind Race) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<HealthComp>(Entity);
  Reg.emplace<WanderAIComp>(Entity);
  Reg.emplace<TileComp>(Entity, T);
  Reg.emplace<NameComp>(Entity, Name);
  Reg.emplace<LineOfSightComp>(Entity);
  Reg.emplace<AttackAIComp>(Entity);
  Reg.emplace<FactionComp>(Entity, Faction);
  Reg.emplace<RaceComp>(Entity, Race);
  Reg.emplace<AgilityComp>(Entity);

  // FIXME make this optional depending on enemy kind
  Reg.emplace<InventoryComp>(Entity).Inv = I;
  Reg.emplace<StatsComp>(Entity).Base = Stats;
  Reg.emplace<EquipmentComp>(Entity);

  Reg.emplace<CollisionComp>(Entity);
  Reg.emplace<VisibleComp>(Entity);

  // Try equipping items
  InventoryHandler InvHandler(Entity, Reg);
  InvHandler.autoEquipItems();
}

void createHostileCreature(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                           const std::string &Name, const Inventory &I,
                           const StatPoints &Stats) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<HealthComp>(Entity);
  Reg.emplace<WanderAIComp>(Entity);
  Reg.emplace<TileComp>(Entity, T);
  Reg.emplace<NameComp>(Entity, Name);
  Reg.emplace<LineOfSightComp>(Entity);
  Reg.emplace<AttackAIComp>(Entity);
  Reg.emplace<FactionComp>(Entity, FactionKind::Nature);
  Reg.emplace<AgilityComp>(Entity);
  Reg.emplace<StatsComp>(Entity).Base = Stats;
  Reg.emplace<InventoryComp>(Entity).Inv = I;

  Reg.emplace<CollisionComp>(Entity);
  Reg.emplace<VisibleComp>(Entity);
}

void createWorldEntry(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                      const std::string &LevelName) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<TileComp>(Entity, T);
  Reg.emplace<InteractableComp>(
      Entity,
      Interaction{
          "Enter dungeon",
          [LevelName, Entity](auto &EHC, auto SrcEt, auto &) {
            EHC.publish(SwitchGameWorldEvent{{}, LevelName, SrcEt, Entity});
          }});

  Reg.emplace<CollisionComp>(Entity);
  Reg.emplace<VisibleComp>(Entity);
}

void createLevelEntryExit(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                          bool IsExit, int LevelId) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<TileComp>(Entity, T);
  Reg.emplace<InteractableComp>(
      Entity,
      Interaction{IsExit ? "Previous Level" : "Next level",
                  [LevelId, IsExit, Entity](auto &EHC, auto SrcEt, auto &) {
                    EHC.publish(SwitchLevelEvent{
                        {}, LevelId, /*ToEntry=*/!IsExit, SrcEt, Entity});
                  }});
  if (IsExit) {
    Reg.emplace<LevelStartComp>(Entity, LevelId);
  } else {
    Reg.emplace<LevelEndComp>(Entity, LevelId);
  }

  Reg.emplace<CollisionComp>(Entity);
  Reg.emplace<VisibleComp>(Entity);
}

void createChestEntity(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                       const Inventory &I) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<TileComp>(Entity, T);

  // Copy inventory
  auto &Inv = Reg.emplace<InventoryComp>(Entity).Inv;
  Inv = I;

  Reg.emplace<InteractableComp>(
      Entity,
      Interaction{"Open Chest", [Entity](auto &EHC, auto Et, auto &Reg) {
                    EHC.publish(LootEvent{{}, Et, Entity, &Reg});
                  }});

  Reg.emplace<CollisionComp>(Entity);
  Reg.emplace<VisibleComp>(Entity);
}

void createDropEntity(entt::registry &Reg, ymir::Point2d<int> Pos,
                      const Inventory &I) {
  static constexpr Tile DropTile{{'o', cxxg::types::RgbColor{120, 90, 40}}};

  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<TileComp>(Entity, DropTile, -1);
  auto &Inv = Reg.emplace<InventoryComp>(Entity).Inv;
  Inv = I;

  Reg.emplace<InteractableComp>(
      Entity, Interaction{"Loot", [Entity](auto &EHC, auto Et, auto &Reg) {
                            EHC.publish(LootEvent{{}, Et, Entity, &Reg});
                          }});

  Reg.emplace<VisibleComp>(Entity);
}

void createHealerEntity(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<TileComp>(Entity, T);

  Reg.emplace<InteractableComp>(
      Entity,
      Interaction{"Heal", [](auto &EHC, auto Et, auto &Reg) {
                    auto &HC = Reg.template get<HealthComp>(Et);
                    HC.Value = HC.MaxValue;
                    EHC.publish(PlayerInfoMessageEvent() << "You feel better.");
                  }});

  Reg.emplace<CollisionComp>(Entity);
  Reg.emplace<VisibleComp>(Entity);
}

void createShopEntity(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<TileComp>(Entity, T);

  Reg.emplace<InteractableComp>(
      Entity,
      Interaction{"Shop", [](auto &EHC, auto Et, auto &Reg) {
                    EHC.publish(PlayerInfoMessageEvent() << "Shop not implemented yet.");
                    (void)Et;
                    (void)Reg;
                    //EHC.publish(ShopOpenEvent{{}, Et, Entity, &Reg});
                  }});

  Reg.emplace<CollisionComp>(Entity);
  Reg.emplace<VisibleComp>(Entity);
}

void createWorkbenchEntity(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<TileComp>(Entity, T);

  Reg.emplace<InteractableComp>(
      Entity,
      Interaction{"Workbench", [](auto &EHC, auto Et, auto &Reg) {
                    EHC.publish(PlayerInfoMessageEvent() << "Workbench not implemented yet.");
                    //EHC.publish(WorkbenchUseEvent{{}, Et, Entity, &Reg});
                    (void)Et;
                    (void)Reg;
                  }});

  Reg.emplace<CollisionComp>(Entity);
  Reg.emplace<VisibleComp>(Entity);
}

} // namespace rogue