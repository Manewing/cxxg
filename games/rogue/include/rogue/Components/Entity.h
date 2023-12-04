#ifndef ROGUE_COMPONENTS_ENTITY_H
#define ROGUE_COMPONENTS_ENTITY_H

#include <entt/entt.hpp>
#include <filesystem>
#include <rogue/Components/RaceFaction.h>
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
} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_ENTITY_H