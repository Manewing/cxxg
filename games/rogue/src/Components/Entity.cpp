#include <rogue/Components/AI.h>
#include <rogue/Components/Entity.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/Level.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Context.h>
#include <rogue/Game.h>

namespace rogue {

void createEnemy(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
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
  Reg.emplace<MeleeAttackComp>(Entity, 1.0);
  Reg.emplace<FactionComp>(Entity, T.kind() == 't' ? FactionKind::Nature
                                                   : FactionKind::Enemy);
  Reg.emplace<AgilityComp>(Entity);
  Reg.emplace<CollisionComp>(Entity);

  // FIXME make this optional depending on enemy kind
  Reg.emplace<InventoryComp>(Entity).Inv = I;
  Reg.emplace<StatsComp>(Entity).Base = Stats;
  Reg.emplace<EquipmentComp>(Entity);

  // FIXME this should be part of an enemy/NPC AI system
  // Try equipping items
  auto &Inv = Reg.get<InventoryComp>(Entity).Inv;
  auto &Equip = Reg.get<EquipmentComp>(Entity).Equip;
  for (std::size_t Idx = 0; Idx < Inv.size(); Idx++) {
    const auto &It = Inv.getItem(Idx);
    if ((It.getType() & ItemType::EquipmentMask) != ItemType::None &&
        Equip.canEquip(It.getType()) && It.getMaxStackSize() == 1 &&
        It.canApplyTo(Entity, Reg, CapabilityFlags::EquipOn)) {
      It.applyTo(Entity, Reg, CapabilityFlags::EquipOn);
      Equip.equip(Inv.takeItem(Idx));
      Idx -= 1;
    }
  }
}

void createLevelEntryExit(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                          bool IsExit, int LevelId) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<TileComp>(Entity, T);
  Reg.emplace<InteractableComp>(
      Entity, Interaction{IsExit ? "Previous Level" : "Next level",
                          [LevelId, IsExit](auto &G, auto, auto &) {
                            G.switchLevel(LevelId, /*ToEntry=*/!IsExit);
                          }});
  Reg.emplace<CollisionComp>(Entity);
  if (IsExit) {
    Reg.emplace<LevelStartComp>(Entity, LevelId);
  } else {
    Reg.emplace<LevelEndComp>(Entity, LevelId);
  }
}

void createChestEntity(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                       const Inventory &I) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<TileComp>(Entity, T);
  Reg.emplace<ChestComp>(Entity);
  Reg.emplace<CollisionComp>(Entity);

  // Copy inventory
  auto &Inv = Reg.get<ChestComp>(Entity).Inv;
  Inv = I;

  Reg.emplace<InteractableComp>(
      Entity, Interaction{"Open Chest", [&Inv](auto &G, auto Et, auto &Reg) {
                            G.UICtrl.setLootUI(Inv, Et, Reg);
                          }});
}

void createDropEntity(entt::registry &Reg, ymir::Point2d<int> Pos,
                      const Inventory &I) {
  static constexpr Tile DropTile{{'o', cxxg::types::RgbColor{120, 90, 40}}};

  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<TileComp>(Entity, DropTile, -1);
  Reg.emplace<DropComp>(Entity);

  auto &Inv = Reg.get<DropComp>(Entity).Inv;
  Inv = I;

  Reg.emplace<InteractableComp>(
      Entity, Interaction{"Loot", [&Inv](auto &G, auto Et, auto &Reg) {
                            G.UICtrl.setLootUI(Inv, Et, Reg);
                          }});
}

} // namespace rogue