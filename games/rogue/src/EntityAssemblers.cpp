#include <rogue/EntityAssemblers.h>
#include <rogue/Event.h>
#include <rogue/InventoryHandler.h>
#include <rogue/ItemDatabase.h>

namespace rogue {

TileCompAssembler::TileCompAssembler(Tile T) : T(T) {}

void TileCompAssembler::assemble(entt::registry &Reg,
                                 entt::entity Entity) const {
  Reg.emplace<TileComp>(Entity, T);
}

FactionCompAssembler::FactionCompAssembler(FactionKind F) : F(F) {}

void FactionCompAssembler::assemble(entt::registry &Reg,
                                    entt::entity Entity) const {
  Reg.emplace<FactionComp>(Entity, F);
}

RaceCompAssembler::RaceCompAssembler(RaceKind R) : R(R) {}

void RaceCompAssembler::assemble(entt::registry &Reg,
                                 entt::entity Entity) const {
  Reg.emplace<RaceComp>(Entity, R);
}

InventoryCompAssembler::InventoryCompAssembler(const ItemDatabase &ItemDb,
                                               const std::string &LootTable,
                                               bool IsPersistent, bool IsLooted)
    : ItemDb(ItemDb), LootTable(LootTable), IsPersistent(IsPersistent),
      IsLooted(IsLooted) {}

namespace {

Inventory generateLootInventory(const ItemDatabase &ItemDb,
                                const std::string &LootTable) {
  const auto &LtCt = ItemDb.getLootTable(LootTable);
  auto Loot = LtCt->generateLoot();

  Inventory Inv;
  for (const auto &Rw : Loot) {
    auto It = ItemDb.createItem(Rw.ItId, Rw.Count);
    Inv.addItem(It);
  }
  return Inv;
}

} // namespace

void InventoryCompAssembler::assemble(entt::registry &Reg,
                                      entt::entity Entity) const {
  auto Inv = generateLootInventory(ItemDb, LootTable);
  Reg.emplace<InventoryComp>(Entity, Inv, IsPersistent, IsLooted);
}

bool AutoEquipAssembler::isPostProcess() const { return true; }

void AutoEquipAssembler::assemble(entt::registry &Reg,
                                  entt::entity Entity) const {
  InventoryHandler InvHdlr(Entity, Reg);
  InvHdlr.autoEquipItems();
}

ChestInteractableCompAssembler::ChestInteractableCompAssembler(Tile LootedTile)
    : LootedTile(LootedTile) {}

namespace {

void handleLootChest(entt::registry &Reg, const entt::entity &Entity,
                     const entt::entity &ActEt, EventHubConnector &EHC) {
  auto &TC = Reg.template get<TileComp>(Entity);
  auto &IC = Reg.template get<InventoryComp>(Entity);
  if (!IC.Looted) {
    if (auto *Rgb = std::get_if<cxxg::types::RgbColor>(&TC.T.color())) {
      Rgb->R *= 0.5;
      Rgb->G *= 0.5;
      Rgb->B *= 0.5;
    }
    IC.Looted = true;
  }

  EHC.publish(LootEvent{{}, "Chest", ActEt, Entity, &Reg});
}

} // namespace

void ChestInteractableCompAssembler::assemble(entt::registry &Reg,
                                              entt::entity Entity) const {
  (void)LootedTile; // FIXME
  Reg.emplace<InteractableComp>(
      Entity,
      Interaction{"Open Chest", [Entity](auto &EHC, auto Et, auto &Reg) {
                    handleLootChest(Reg, Entity, Et, EHC);
                  }});
}

DoorCompAssembler::DoorCompAssembler(bool IsOpen, Tile OpenTile,
                                     Tile ClosedTile, std::optional<int> KeyId)
    : IsOpen(IsOpen), OpenTile(OpenTile), ClosedTile(ClosedTile), KeyId(KeyId) {
}

namespace {

bool unlockDoor(entt::registry &Reg, const entt::entity &DoorEt,
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

  Reg.get<InteractableComp>(DoorEt).Action.Msg = "Open door";

  return true;
}

void openDoor(entt::registry &Reg, const entt::entity &Entity) {
  Reg.get<DoorComp>(Entity).IsOpen = true;
  Reg.erase<CollisionComp>(Entity);
  Reg.erase<BlocksLOS>(Entity);
  Reg.get<InteractableComp>(Entity).Action.Msg = "Close door";

  // FIXME use open/closed door tiles
  auto &T = Reg.get<TileComp>(Entity);
  T.ZIndex = -2;
  T.T.T.Char = '/';
}

void closeDoor(entt::registry &Reg, const entt::entity &Entity) {
  Reg.get<DoorComp>(Entity).IsOpen = false;
  Reg.emplace<CollisionComp>(Entity);
  Reg.emplace<BlocksLOS>(Entity);
  Reg.get<InteractableComp>(Entity).Action.Msg = "Open door";

  // FIXME use open/closed door tiles
  auto &T = Reg.get<TileComp>(Entity);
  T.ZIndex = 0;
  T.T.T.Char = '+';
}

} // namespace

void DoorCompAssembler::assemble(entt::registry &Reg,
                                 entt::entity Entity) const {
  auto &DC = Reg.emplace<DoorComp>(Entity);
  DC.IsOpen = IsOpen;
  DC.KeyId = KeyId;

  const char *InteractMsg =
      DC.isLocked() ? "Unlock door" : (IsOpen ? "Close door" : "XOpen door");
  Reg.emplace<InteractableComp>(
      Entity,
      Interaction{
          InteractMsg, [Entity](auto &EHC, auto Et, auto &Reg) {
            auto &DC = Reg.template get<DoorComp>(Entity);
            if (DC.isLocked()) {
              if (unlockDoor(Reg, Entity, Et)) {
                EHC.publish(PlayerInfoMessageEvent() << "You unlock the door.");
              } else {
                EHC.publish(PlayerInfoMessageEvent()
                            << "You fail to unlock the door.");
              }
            } else if (DC.IsOpen) {
              closeDoor(Reg, Entity);
              EHC.publish(PlayerInfoMessageEvent() << "You open the door.");
            } else {
              openDoor(Reg, Entity);
              EHC.publish(PlayerInfoMessageEvent() << "You close the door.");
            }
            (void)Et;
          }});

  auto &TC = Reg.emplace<TileComp>(Entity, IsOpen ? OpenTile : ClosedTile);
  if (!IsOpen) {
    TC.ZIndex = 0;
  } else {
    TC.ZIndex = -2;
  }
}

WorldEntryInteractableCompAssembler::WorldEntryInteractableCompAssembler(
    const std::string &LevelName)
    : LevelName(LevelName) {}

void WorldEntryInteractableCompAssembler::assemble(entt::registry &Reg,
                                                   entt::entity Entity) const {
  Reg.emplace<InteractableComp>(
      Entity,
      Interaction{
          "Enter dungeon", [this, Entity](auto &EHC, auto SrcEt, auto &) {
            EHC.publish(SwitchGameWorldEvent{{}, LevelName, SrcEt, Entity});
          }});
}

LevelEntryExitAssembler::LevelEntryExitAssembler(bool IsExit, int LevelId)
    : IsExit(IsExit), LevelId(LevelId) {}

void LevelEntryExitAssembler::assemble(entt::registry &Reg,
                                       entt::entity Entity) const {
  Reg.emplace<InteractableComp>(
      Entity, Interaction{IsExit ? "Previous Level" : "Next level",
                          [this, &Reg, Entity](auto &EHC, auto SrcEt, auto &) {
                            auto LId = -1;
                            if (IsExit) {
                              LId = Reg.get<LevelStartComp>(Entity).NextLevelId;
                            } else {
                              LId = Reg.get<LevelEndComp>(Entity).NextLevelId;
                            }
                            EHC.publish(SwitchLevelEvent{
                                {}, LId, /*ToEntry=*/!IsExit, SrcEt, Entity});
                          }});
  if (IsExit) {
    Reg.emplace<LevelStartComp>(Entity, LevelId);
  } else {
    Reg.emplace<LevelEndComp>(Entity, LevelId);
  }
}

void HealerInteractableCompAssembler::assemble(entt::registry &Reg,
                                               entt::entity Entity) const {
  Reg.emplace<InteractableComp>(
      Entity, Interaction{"Heal", [](auto &EHC, auto Et, auto &Reg) {
                            auto &HC = Reg.template get<HealthComp>(Et);
                            HC.Value = HC.MaxValue;
                            EHC.publish(PlayerInfoMessageEvent()
                                        << "You feel better.");
                          }});
}

void ShopAssembler::assemble(entt::registry &Reg, entt::entity Entity) const {
  Reg.emplace<InteractableComp>(
      Entity, Interaction{"Shop", [](auto &EHC, auto Et, auto &Reg) {
                            EHC.publish(PlayerInfoMessageEvent()
                                        << "Shop not implemented yet.");
                            (void)Et;
                            (void)Reg;
                            // EHC.publish(ShopOpenEvent{{}, Et, Entity, &Reg});
                          }});
}

void WorkbenchAssembler::assemble(entt::registry &Reg,
                                  entt::entity Entity) const {
  Reg.emplace<InteractableComp>(
      Entity, Interaction{"Workbench", [](auto &EHC, auto Et, auto &Reg) {
                            EHC.publish(PlayerInfoMessageEvent()
                                        << "Workbench not implemented yet.");
                            (void)Et;
                            (void)Reg;
                            // EHC.publish(WorkbenchOpenEvent{{}, Et, Entity,
                            // &Reg});
                          }});
}

StatsCompAssembler::StatsCompAssembler(StatPoints Stats) : Stats(Stats) {}

void StatsCompAssembler::assemble(entt::registry &Reg,
                                  entt::entity Entity) const {
  Reg.emplace<StatsComp>(Entity).Base = Stats;
}

} // namespace rogue