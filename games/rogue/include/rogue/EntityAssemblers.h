#ifndef ROGUE_ENTITY_ASSEMBLERS_H
#define ROGUE_ENTITY_ASSEMBLERS_H

#include <entt/entt.hpp>
#include <rogue/Components/AI.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/LOS.h>
#include <rogue/Components/Level.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/RaceFaction.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/EntityDatabase.h>
#include <ymir/Types.hpp>

namespace rogue {

template <typename T>
class DefaultConstructEntityAssembler : public EntityAssembler {
public:
  void assemble(entt::registry &Reg, entt::entity Entity) const override {
    Reg.emplace<T>(Entity);
  }
};

class TileCompAssembler : public EntityAssembler {
public:
  explicit TileCompAssembler(Tile T);
  void assemble(entt::registry &Reg, entt::entity Entity) const override;

private:
  Tile T;
};

class FactionCompAssembler : public EntityAssembler {
public:
  explicit FactionCompAssembler(FactionKind F);
  void assemble(entt::registry &Reg, entt::entity Entity) const override;

private:
  FactionKind F;
};

class RaceCompAssembler : public EntityAssembler {
public:
  explicit RaceCompAssembler(RaceKind R);
  void assemble(entt::registry &Reg, entt::entity Entity) const override;

private:
  RaceKind R;
};

class InventoryCompAssembler : public EntityAssembler {
public:
  InventoryCompAssembler(const ItemDatabase &ItemDb,
                         const std::string &LootTable);
  void assemble(entt::registry &Reg, entt::entity Entity) const override;

private:
  const ItemDatabase &ItemDb;
  std::string LootTable;
};

class AutoEquipAssembler : public EntityAssembler {
public:
  bool isPostProcess() const override;
  void assemble(entt::registry &Reg, entt::entity Entity) const override;
};

class DoorCompAssembler : public EntityAssembler {
public:
  DoorCompAssembler(bool IsOpen, Tile OpenTile, Tile ClosedTile,
                    std::optional<int> KeyId);
  void assemble(entt::registry &Reg, entt::entity Entity) const override;

private:
  bool IsOpen;
  Tile OpenTile;
  Tile ClosedTile;
  std::optional<int> KeyId;
};

class LootedInteractCompAssembler : public EntityAssembler {
public:
  explicit LootedInteractCompAssembler(bool IsLooted, bool IsPersistent,
                                       Tile DefaultTile, Tile LootedTile,
                                       const std::string &InteractText,
                                       const std::string &LootName);
  void assemble(entt::registry &Reg, entt::entity Entity) const override;

private:
  bool IsLooted;
  bool IsPersistent;
  Tile DefaultTile;
  Tile LootedTile;
  std::string InteractText;
  std::string LootName;
};

class WorldEntryInteractableCompAssembler : public EntityAssembler {
public:
  explicit WorldEntryInteractableCompAssembler(const std::string &LevelName);
  void assemble(entt::registry &Reg, entt::entity Entity) const override;

private:
  std::string LevelName;
};

class LevelEntryExitAssembler : public EntityAssembler {
public:
  LevelEntryExitAssembler(bool IsExit, int LevelId);
  void assemble(entt::registry &Reg, entt::entity Entity) const override;

private:
  bool IsExit;
  int LevelId;
};

class HealerInteractableCompAssembler : public EntityAssembler {
public:
  void assemble(entt::registry &Reg, entt::entity Entity) const override;
};

class ShopAssembler : public EntityAssembler {
public:
  void assemble(entt::registry &Reg, entt::entity Entity) const override;
};

class WorkbenchAssembler : public EntityAssembler {
public:
  void assemble(entt::registry &Reg, entt::entity Entity) const override;
};

class StatsCompAssembler : public EntityAssembler {
public:
  explicit StatsCompAssembler(StatPoints Stats);
  void assemble(entt::registry &Reg, entt::entity Entity) const override;

  inline const StatPoints &getStats() const { return Stats; }

private:
  StatPoints Stats;
};

// Keep sorted
using AttackAICompAssembler = DefaultConstructEntityAssembler<AttackAIComp>;
using BlockLOSCompAssembler = DefaultConstructEntityAssembler<BlocksLOS>;
using CollisionCompAssembler = DefaultConstructEntityAssembler<CollisionComp>;
using EquipmentCompAssembler = DefaultConstructEntityAssembler<EquipmentComp>;
using HealthCompAssembler = DefaultConstructEntityAssembler<HealthComp>;
using PlayerCompAssembler = DefaultConstructEntityAssembler<PlayerComp>;
using PositionCompAssembler = DefaultConstructEntityAssembler<PositionComp>;
using VisibleCompAssembler = DefaultConstructEntityAssembler<VisibleComp>;
using WanderAICompAssembler = DefaultConstructEntityAssembler<WanderAIComp>;

// TODO make configurable
using LineOfSightCompAssembler =
    DefaultConstructEntityAssembler<LineOfSightComp>;
using AgilityCompAssembler = DefaultConstructEntityAssembler<AgilityComp>;

} // namespace rogue

#endif // #ifndef ROGUE_ENTITY_ASSEMBLERS_H