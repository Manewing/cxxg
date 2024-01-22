#ifndef ROGUE_COMPONENTS_ENTITY_H
#define ROGUE_COMPONENTS_ENTITY_H

#include <entt/entt.hpp>
#include <filesystem>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Combat.h>
#include <rogue/Tile.h>
#include <string>
#include <ymir/Types.hpp>

namespace rogue {
class Inventory;
struct StatPoints;
} // namespace rogue

namespace rogue {

void createDropEntity(entt::registry &Reg, ymir::Point2d<int> Pos,
                      const Inventory &Inv);

void createTempDamage(entt::registry &Reg, const DamageComp &DC,
                      ymir::Point2d<int> Pos, Tile T);

void createProjectile(entt::registry &Reg, const DamageComp &DC,
                      ymir::Point2d<int> Pos, ymir::Point2d<int> TargetPos,
                      StatValue Agility = 100);

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_ENTITY_H