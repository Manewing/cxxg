#include <rogue/Components/AI.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Entity.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/LOS.h>
#include <rogue/Components/Level.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/RaceFaction.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Context.h>
#include <rogue/Event.h>
#include <rogue/EventHub.h>
#include <rogue/InventoryHandler.h>

namespace rogue {

void createDropEntity(entt::registry &Reg, ymir::Point2d<int> Pos,
                      const Inventory &I) {
  static constexpr Tile DropTile{{'o', cxxg::types::RgbColor{120, 90, 40}}};

  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<TileComp>(Entity, DropTile, -1);
  Reg.emplace<NameComp>(Entity, "Drop");

  // Copy inventory
  auto &IC = Reg.emplace<InventoryComp>(Entity);
  IC.Inv = I;

  auto &LIC = Reg.emplace<LootInteractComp>(Entity);
  LIC.IsLooted = false;
  LIC.IsPersistent = false;

  Reg.emplace<InteractableComp>(
      Entity,
      Interaction{"Loot", [Entity](auto &EHC, auto Et, auto &Reg) {
                    EHC.publish(LootEvent{{}, "Loot", Et, Entity, &Reg});
                  }});

  Reg.emplace<VisibleComp>(Entity);
}

} // namespace rogue