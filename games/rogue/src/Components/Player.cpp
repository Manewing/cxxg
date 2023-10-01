#include <rogue/Components/AI.h>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Helpers.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/Level.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>

namespace rogue {
namespace {

using PlayerCompList =
    ComponentList<TileComp, FactionComp, PlayerComp, PositionComp, StatsComp,
                  HealthComp, NameComp, LineOfSightComp, AgilityComp,
                  MeleeAttackComp, InventoryComp, EquipmentComp, CollisionComp>;

entt::entity createPlayer(entt::registry &Reg, const PlayerComp &PC,
                          const PositionComp &PosComp, const StatsComp &Stats,
                          const HealthComp &HC, const NameComp &NC,
                          const LineOfSightComp LOSC, const AgilityComp &AC,
                          const MeleeAttackComp &MC,
                          const InventoryComp &InvComp,
                          const EquipmentComp &EquipComp) {
  static constexpr Tile PlayerTile{{'@', cxxg::types::RgbColor{255, 255, 50}}};

  auto Entity = Reg.create();

  // Fixed values
  Reg.emplace<TileComp>(Entity, PlayerTile);
  Reg.emplace<FactionComp>(Entity, FactionKind::Player);

  // Copy values
  Reg.emplace<PlayerComp>(Entity, PC);
  Reg.emplace<PositionComp>(Entity, PosComp);
  Reg.emplace<StatsComp>(Entity, Stats);
  Reg.emplace<HealthComp>(Entity, HC);
  Reg.emplace<NameComp>(Entity, NC);
  Reg.emplace<LineOfSightComp>(Entity, LOSC);
  Reg.emplace<AgilityComp>(Entity, AC);
  Reg.emplace<MeleeAttackComp>(Entity, MC);
  Reg.emplace<InventoryComp>(Entity, InvComp);
  Reg.emplace<EquipmentComp>(Entity, EquipComp);
  Reg.emplace<CollisionComp>(Entity);

  return Entity;
}

} // namespace

entt::entity PlayerComp::createPlayer(entt::registry &Reg,
                                      const std::string &Name,
                                      ymir::Point2d<int> Pos) {
  return ::rogue::createPlayer(Reg, PlayerComp{}, PositionComp{Pos},
                               StatsComp{StatPoints{1, 1, 10, 10}, {}},
                               HealthComp{}, NameComp{Name}, LineOfSightComp{},
                               AgilityComp{}, MeleeAttackComp{},
                               InventoryComp{}, EquipmentComp{});
}

entt::entity PlayerComp::copyPlayer(entt::registry &RegFrom,
                                    entt::registry &RegTo) {
  entt::entity PlayerEntity = entt::null;
  auto View = RegFrom.view<PlayerComp>();
  bool Found = false;
  for (auto Entity : View) {
    assert(!Found && "Multiple players found");
    Found = true;
    PlayerEntity = RegTo.create();
    copyComponentsOrFail<PlayerCompList>(Entity, RegFrom, PlayerEntity, RegTo);
    copyBuffs(Entity, RegFrom, PlayerEntity, RegTo);
  }
  return PlayerEntity;
}

entt::entity PlayerComp::movePlayer(entt::registry &RegFrom,
                                    entt::registry &RegTo) {
  entt::entity PlayerEntity = entt::null;
  removePlayer(RegTo);
  PlayerEntity = copyPlayer(RegFrom, RegTo);
  removePlayer(RegFrom);
  return PlayerEntity;
}

void PlayerComp::removePlayer(entt::registry &Reg) {
  auto View = Reg.view<PlayerComp>();
  for (auto Entity : View) {
    Reg.destroy(Entity);
  }
}

} // namespace rogue