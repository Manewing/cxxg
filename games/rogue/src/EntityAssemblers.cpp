#include <rogue/EntityAssemblers.h>

namespace rogue {

void TileCompAssembler::assemble(entt::registry &Reg,
                                 entt::entity Entity) const {
  Reg.emplace<Tile>(Entity, T);
}

} // namespace rogue