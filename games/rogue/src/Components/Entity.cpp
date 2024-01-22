#include <rogue/Components/Entity.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/LOS.h>
#include <rogue/Components/Level.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Context.h>
#include <rogue/Event.h>
#include <rogue/EventHub.h>

namespace rogue {

void createDropEntity(entt::registry &Reg, ymir::Point2d<int> Pos,
                      const Inventory &I) {
  static constexpr Tile DropTile{{'o', cxxg::types::RgbColor{120, 90, 40}}};

  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<TileComp>(Entity, DropTile, -1);
  Reg.emplace<NameComp>(Entity, "Drop", "Drop containing items");

  // Copy inventory
  auto &IC = Reg.emplace<InventoryComp>(Entity);
  IC.Inv = I;

  auto &LIC = Reg.emplace<LootInteractComp>(Entity);
  LIC.IsLooted = false;
  LIC.IsPersistent = false;

  auto &ITC = Reg.get_or_emplace<InteractableComp>(Entity);
  ITC.Actions.push_back({"Loot", [Entity](auto &EHC, auto Et, auto &Reg) {
                           Reg.template get<LootInteractComp>(Entity).IsLooted = true;
                           EHC.publish(LootEvent{{}, "Loot", Et, Entity, &Reg});
                         }});

  Reg.emplace<VisibleComp>(Entity);
}

void createTempDamage(entt::registry &Reg, const DamageComp &DC,
                      ymir::Point2d<int> Pos, Tile T) {
  auto E = Reg.create();
  Reg.emplace<DamageComp>(E, DC).Ticks = 1;
  Reg.emplace<PositionComp>(E, Pos);
  Reg.emplace<TileComp>(E, T);
  Reg.emplace<NameComp>(E, "Damage", "Damage");
  Reg.emplace<VisibleComp>(E);
}

void createProjectile(entt::registry &Reg, const DamageComp &DC,
                      ymir::Point2d<int> Pos, ymir::Point2d<int> TargetPos,
                      StatValue Agility) {
  static constexpr auto ProjectileTile =
      Tile{{'*', cxxg::types::RgbColor{255, 65, 0}}};

  auto E = Reg.create();
  Reg.emplace<DamageComp>(E, DC);
  Reg.emplace<PositionComp>(E, Pos);
  Reg.emplace<AgilityComp>(E, Agility);
  Reg.emplace<TileComp>(E, ProjectileTile);
  Reg.emplace<NameComp>(E, "Projectile", "Projectile");
  VectorMovementComp VMC;
  VMC.Flying = true;
  VMC.KillOnWall = true;
  VMC.Vector = (TargetPos - Pos).to<float>();
  VMC.LastPos = Pos.to<float>();
  Reg.emplace<VectorMovementComp>(E, VMC);
  Reg.emplace<VisibleComp>(E);
}

} // namespace rogue