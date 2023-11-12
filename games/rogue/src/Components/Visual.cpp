#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>

namespace rogue {

entt::entity createCursor(entt::registry &Reg, ymir::Point2d<int> Pos) {
  auto CursorEt = Reg.create();
  Reg.emplace<PositionComp>(CursorEt, Pos);
  Reg.emplace<TileComp>(CursorEt, Tile{{'X', cxxg::types::RgbColor{255, 0, 0}}},
                        3000);
  Reg.emplace<CursorComp>(CursorEt);
  return CursorEt;
}

} // namespace rogue