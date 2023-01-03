#include "Components/Entity.h"
#include "Components/AI.h"
#include "Components/Level.h"
#include "Components/Stats.h"
#include "Components/Transform.h"
#include "Components/Visual.h"

void createEnemy(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                 const std::string &Name) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<HealthComp>(Entity);
  Reg.emplace<WanderAIComp>(Entity);
  Reg.emplace<TileComp>(Entity, T);
  Reg.emplace<NameComp>(Entity, Name);
  Reg.emplace<LineOfSightComp>(Entity);
  Reg.emplace<AttackAIComp>(Entity);
  Reg.emplace<MeleeAttackComp>(Entity, T.kind() == 't' ? 60U : 30U);
  //  reg.emplace<MeleeAttack>(Entity, (Fac == 0 ? 60U : 20U));

  // DEBUG
  // Reg.emplace<FactionComp>(Entity, FactionKind::Enemy);
  Reg.emplace<FactionComp>(Entity, T.kind() == 't' ? FactionKind::Nature
                                                   : FactionKind::Enemy);

  Reg.emplace<AgilityComp>(Entity);
}

void createLevelEntryExit(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                          bool IsExit) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<TileComp>(Entity, T);
  if (IsExit) {
    Reg.emplace<LevelEndComp>(Entity);
  } else {
    Reg.emplace<LevelStartComp>(Entity);
  }
}