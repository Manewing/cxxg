#ifndef ROGUE_COMPONENTS_ENTITY_H
#define ROGUE_COMPONENTS_ENTITY_H

#include <entt/entt.hpp>
#include <rogue/Tile.h>
#include <string>
#include <ymir/Types.hpp>

namespace rogue {

void createEnemy(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                 const std::string &Name);

void createLevelEntryExit(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                          bool IsExit, int LevelId);

void createChestEntity(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T);

void createDropEntity(entt::registry &Reg, ymir::Point2d<int> Pos);

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_ENTITY_H