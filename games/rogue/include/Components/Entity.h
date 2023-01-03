#ifndef ROGUE_COMPONENTS_ENTITY_H
#define ROGUE_COMPONENTS_ENTITY_H

#include "Tile.h"
#include <entt/entt.hpp>
#include <string>
#include <ymir/Types.hpp>

void createEnemy(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                 const std::string &Name);

void createLevelEntryExit(entt::registry &Reg, ymir::Point2d<int> Pos, Tile T,
                          bool IsExit);

#endif // #ifndef ROGUE_COMPONENTS_ENTITY_H