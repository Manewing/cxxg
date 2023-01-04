#include "Components/Entity.h"
#include "Components/AI.h"
#include "Components/Level.h"
#include "Components/Player.h"
#include "Components/Stats.h"
#include "Components/Transform.h"
#include "Components/Visual.h"

#include "Game.h"

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
}

void createLevelEntryExit(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                          bool IsExit, int LevelId) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<TileComp>(Entity, T);
  Reg.emplace<InteractableComp>(
      Entity, Interaction{IsExit ? "Previous Level" : "Next level",
                          [LevelId](Game &G) { G.switchLevel(LevelId); }});
  if (IsExit) {
    Reg.emplace<LevelStartComp>(Entity, LevelId);
  } else {
    Reg.emplace<LevelEndComp>(Entity, LevelId);
  }
}