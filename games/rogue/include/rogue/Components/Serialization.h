#ifndef ROGUE_COMPONENTS_SERIALIZATION_H
#define ROGUE_COMPONENTS_SERIALIZATION_H

#include <cstdint>

namespace rogue::serialize {

// Indicates an entity that should be serialized
struct IdComp {
  std::size_t Id = 0;
};

} // namespace rogue::serialize

#endif // #define ROGUE_COMPONENTS_SERIALIZATION_H