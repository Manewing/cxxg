#ifndef ROGUE_COMPONENTS_ENTITY_H
#define ROGUE_COMPONENTS_ENTITY_H

#include <entt/entt.hpp>
#include <rogue/Tile.h>
#include <string>
#include <ymir/Types.hpp>

namespace rogue {
class Inventory;
struct StatPoints;
} // namespace rogue

namespace rogue {

void createEnemy(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                 const std::string &Name, const Inventory &Inv,
                 const StatPoints &Stats);

void createLevelEntryExit(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                          bool IsExit, int LevelId);

void createChestEntity(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                       const Inventory &Inv);

void createDropEntity(entt::registry &Reg, ymir::Point2d<int> Pos,
                      const Inventory &Inv);

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_ENTITY_H