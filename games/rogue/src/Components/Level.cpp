#include <rogue/Components/Items.h>
#include <rogue/Components/LOS.h>
#include <rogue/Components/Level.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Transform.h>

namespace rogue {

bool DoorComp::unlockDoor(entt::registry &Reg, const entt::entity &DoorEt,
                          const entt::entity &ActEt) {
  auto *IC = Reg.try_get<InventoryComp>(ActEt);
  if (!IC) {
    return false;
  }

  auto &DC = Reg.get<DoorComp>(DoorEt);
  if (!DC.hasLock()) {
    return false;
  }

  auto KeyIdx = IC->Inv.getItemIndexForId(DC.KeyId.value());
  if (!KeyIdx) {
    return false;
  }
  (void)IC->Inv.takeItem(KeyIdx.value());
  DC.KeyId = {};

  Reg.get<InteractableComp>(DoorEt).Actions.at(DC.ActionIdx).Msg = "Open door";

  return true;
}

void DoorComp::openDoor(entt::registry &Reg, const entt::entity &Entity) {
  auto &DC = Reg.get<DoorComp>(Entity);
  DC.IsOpen = true;
  if (Reg.any_of<CollisionComp>(Entity)) {
    Reg.erase<CollisionComp>(Entity);
  }
  if (Reg.any_of<BlocksLOS>(Entity)) {
    Reg.erase<BlocksLOS>(Entity);
  }

  Reg.get<InteractableComp>(Entity).Actions.at(DC.ActionIdx).Msg = "Close door";

  auto &T = Reg.get<TileComp>(Entity);
  T.ZIndex = -2;
  T.T = DC.OpenTile;
}

void DoorComp::closeDoor(entt::registry &Reg, const entt::entity &Entity) {
  auto &DC = Reg.get<DoorComp>(Entity);
  DC.IsOpen = false;
  Reg.emplace_or_replace<CollisionComp>(Entity);
  Reg.emplace_or_replace<BlocksLOS>(Entity);
  Reg.get<InteractableComp>(Entity).Actions.at(DC.ActionIdx).Msg = "Open door";

  auto &T = Reg.get<TileComp>(Entity);
  T.ZIndex = 0;
  T.T = DC.ClosedTile;
}

} // namespace rogue