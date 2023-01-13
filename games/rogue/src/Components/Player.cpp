#include <rogue/Components/AI.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/Level.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>

namespace rogue {
namespace {

entt::entity createPlayer(entt::registry &Reg, const PlayerComp &PC,
                          const PositionComp &PosComp, const HealthComp &HC,
                          const NameComp &NC, const LineOfSightComp LOSC,
                          const AgilityComp &AC, const MeleeAttackComp &MC,
                          const MovementComp &MVC,
                          const InventoryComp &InvComp) {
  static constexpr Tile PlayerTile{{'@', cxxg::types::RgbColor{255, 255, 50}}};

  auto Entity = Reg.create();

  // Fixed values
  Reg.emplace<TileComp>(Entity, PlayerTile);
  Reg.emplace<FactionComp>(Entity, FactionKind::Player);

  // Copy values
  Reg.emplace<PlayerComp>(Entity, PC);
  Reg.emplace<PositionComp>(Entity, PosComp);
  Reg.emplace<HealthComp>(Entity, HC);
  Reg.emplace<NameComp>(Entity, NC);
  Reg.emplace<LineOfSightComp>(Entity, LOSC);
  Reg.emplace<AgilityComp>(Entity, AC);
  Reg.emplace<MeleeAttackComp>(Entity, MC);
  Reg.emplace<MovementComp>(Entity, MVC);
  Reg.emplace<InventoryComp>(Entity, InvComp);
  Reg.emplace<CollisionComp>(Entity);

  return Entity;
}

} // namespace

entt::entity PlayerComp::createPlayer(entt::registry &Reg,
                                      const std::string &Name,
                                      ymir::Point2d<int> Pos) {
  return ::rogue::createPlayer(Reg, PlayerComp{}, PositionComp{Pos},
                               HealthComp{}, NameComp{Name}, LineOfSightComp{},
                               AgilityComp{}, MeleeAttackComp{}, MovementComp{},
                               InventoryComp{});
}

entt::entity PlayerComp::copyPlayer(entt::registry &RegFrom,
                                    entt::registry &RegTo) {
  entt::entity PlayerEntity = entt::null;
  auto View = RegFrom.view<PlayerComp>();
  for (auto Entity : View) {
    PlayerEntity = ::rogue::createPlayer(
        RegTo, RegFrom.get<PlayerComp>(Entity),
        RegFrom.get<PositionComp>(Entity), RegFrom.get<HealthComp>(Entity),
        RegFrom.get<NameComp>(Entity), RegFrom.get<LineOfSightComp>(Entity),
        RegFrom.get<AgilityComp>(Entity), RegFrom.get<MeleeAttackComp>(Entity),
        RegFrom.get<MovementComp>(Entity), RegFrom.get<InventoryComp>(Entity));
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