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
                 const std::string &Name) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<HealthComp>(Entity, HealthComp{{T.kind() == 't' ? 30.0 : 10.0,
                                              T.kind() == 't' ? 30.0 : 10.0}});
  Reg.emplace<WanderAIComp>(Entity);
  Reg.emplace<TileComp>(Entity, T);
  Reg.emplace<NameComp>(Entity, Name);
  Reg.emplace<LineOfSightComp>(Entity);
  Reg.emplace<AttackAIComp>(Entity);
  Reg.emplace<MeleeAttackComp>(Entity, T.kind() == 't' ? 3.0 : 6.0);
  Reg.emplace<FactionComp>(Entity, T.kind() == 't' ? FactionKind::Nature
                                                   : FactionKind::Enemy);
  Reg.emplace<AgilityComp>(Entity);
  Reg.emplace<CollisionComp>(Entity);
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

namespace {

void generateRandomLoot(Inventory &Inv, ItemDatabase &ItemDb) {
  // TODO what kind of loot is there?
  //  special items based on entity kind (e.g. boss)
  //  common items based on entity level

  // DEBUG ==>
  unsigned NumItems =  rand() % 5 + 1;
  for (unsigned I = 0; I < NumItems; ++I) {
    auto It = ItemDb.createItem(ItemDb.getRandomItemId());
    It.StackSize = rand() % (It.getMaxStackSize() / 2 + 1) + 1;
    Inv.addItem(It);
  }
  // <== DEBUG
}

} // namespace

void createChestEntity(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<TileComp>(Entity, T);
  Reg.emplace<ChestComp>(Entity);
  Reg.emplace<CollisionComp>(Entity);

  auto &Inv = Reg.get<ChestComp>(Entity).Inv;
  generateRandomLoot(Inv, Reg.ctx().get<GameContext>().ItemDb);

  Reg.emplace<InteractableComp>(
      Entity, Interaction{"Open Chest", [&Inv](auto &G, auto Et, auto &Reg) {
                            G.UICtrl.setLootUI(Inv, Et, Reg);
                          }});
}

void createDropEntity(entt::registry &Reg, ymir::Point2d<int> Pos) {
  static constexpr Tile DropTile{{'o', cxxg::types::RgbColor{120, 90, 40}}};

  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<TileComp>(Entity, DropTile, -1);
  Reg.emplace<DropComp>(Entity);

  auto &Inv = Reg.get<DropComp>(Entity).Inv;
  generateRandomLoot(Inv, Reg.ctx().get<GameContext>().ItemDb);

  Reg.emplace<InteractableComp>(
      Entity, Interaction{"Loot", [&Inv](auto &G, auto Et, auto &Reg) {
                            G.UICtrl.setLootUI(Inv, Et, Reg);
                          }});
}

} // namespace rogue