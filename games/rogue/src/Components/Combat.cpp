#include <rogue/Components/Combat.h>
#include <rogue/Components/Helpers.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>

namespace rogue {

// FIXME include collision?
using ProjectileCompList = ComponentList<DamageComp, PositionComp, AgilityComp,
                                         TileComp, MovementComp>;

static constexpr auto ProjectileTile =
    Tile{{'*', cxxg::types::RgbColor{255, 65, 0}}};

void createProjectile(entt::registry &Reg, entt::entity Source,
                      StatValue Damage, int Hits, ymir::Dir2d MoveDir,
                      ymir::Point2d<int> Pos, StatValue Agility) {
  auto E = Reg.create();
  Reg.emplace<DamageComp>(E, Source, Damage, Hits);
  Reg.emplace<PositionComp>(E, Pos);
  Reg.emplace<AgilityComp>(E, Agility);
  Reg.emplace<TileComp>(E, ProjectileTile);
  MovementComp MC;
  MC.Dir = MoveDir;
  MC.Clear = false;
  MC.Flying = true;
  MC.KillOnWall = true;
  Reg.emplace<MovementComp>(E, MC);
}

} // namespace rogue