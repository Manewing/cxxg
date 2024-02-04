#ifndef ROGUE_TYPES_H
#define ROGUE_TYPES_H

#include <rogue/StrongType.h>

namespace rogue {

using StatPoint = int;
using StatValue = double;

ROGUE_STRONG_TYPE(int, CraftingRecipeId);
ROGUE_STRONG_TYPE(int, ItemProtoId);

} // namespace rogue

#endif // #ifndef ROGUE_TYPES_H