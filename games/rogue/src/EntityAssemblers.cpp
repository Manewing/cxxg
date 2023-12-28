#include <rogue/Context.h>
#include <rogue/CraftingHandler.h>
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
                                               unsigned MaxStackSize)
    : ItemDb(ItemDb), LootTable(LootTable), MaxStackSize(MaxStackSize) {}

namespace {

Inventory generateLootInventory(const ItemDatabase &ItemDb,
                                const std::string &LootTable,
                                unsigned MaxStackSize) {
  const auto &LtCt = ItemDb.getLootTable(LootTable);
  auto Loot = LtCt->generateLoot();

  Inventory Inv(MaxStackSize);
  for (const auto &Rw : Loot) {
    auto It = ItemDb.createItem(Rw.ItId, Rw.Count);
    Inv.addItem(It);
  }
  return Inv;
}

} // namespace

void InventoryCompAssembler::assemble(entt::registry &Reg,
                                      entt::entity Entity) const {
  auto Inv = generateLootInventory(ItemDb, LootTable, MaxStackSize);
  Reg.emplace<InventoryComp>(Entity, Inv);
}

bool AutoEquipAssembler::isPostProcess() const { return true; }

void AutoEquipAssembler::assemble(entt::registry &Reg,
                                  entt::entity Entity) const {
  CraftingHandler Crafter;
  InventoryHandler InvHdlr(Entity, Reg, Crafter);
  InvHdlr.autoEquipItems();
}

DoorCompAssembler::DoorCompAssembler(bool IsOpen, Tile OpenTile,
                                     Tile ClosedTile, std::optional<int> KeyId)
    : IsOpen(IsOpen), OpenTile(OpenTile), ClosedTile(ClosedTile), KeyId(KeyId) {
}

namespace {

bool unlockDoor(entt::registry &Reg, const entt::entity &DoorEt,
                const entt::entity &ActEt, std::size_t ActionIdx) {
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

  Reg.get<InteractableComp>(DoorEt).Actions.at(ActionIdx).Msg = "Open door";

  return true;
}

void openDoor(entt::registry &Reg, const entt::entity &Entity,
              std::size_t ActionIdx) {
  auto &DC = Reg.get<DoorComp>(Entity);
  DC.IsOpen = true;
  Reg.erase<CollisionComp>(Entity);
  Reg.erase<BlocksLOS>(Entity);

  Reg.get<InteractableComp>(Entity).Actions.at(ActionIdx).Msg = "Close door";

  auto &T = Reg.get<TileComp>(Entity);
  T.ZIndex = -2;
  T.T = DC.OpenTile;
}

void closeDoor(entt::registry &Reg, const entt::entity &Entity,
               std::size_t ActionIdx) {
  auto &DC = Reg.get<DoorComp>(Entity);
  DC.IsOpen = false;
  Reg.emplace<CollisionComp>(Entity);
  Reg.emplace<BlocksLOS>(Entity);
  Reg.get<InteractableComp>(Entity).Actions.at(ActionIdx).Msg = "Open door";

  auto &T = Reg.get<TileComp>(Entity);
  T.ZIndex = 0;
  T.T = DC.ClosedTile;
}

} // namespace

void DoorCompAssembler::assemble(entt::registry &Reg,
                                 entt::entity Entity) const {
  auto &DC = Reg.emplace<DoorComp>(Entity);
  DC.IsOpen = IsOpen;
  DC.KeyId = KeyId;
  DC.OpenTile = OpenTile;
  DC.ClosedTile = ClosedTile;

  const char *InteractMsg =
      DC.isLocked() ? "Unlock door" : (IsOpen ? "Close door" : "Open door");

  auto &ITC = Reg.get_or_emplace<InteractableComp>(Entity);
  const auto InteractIdx = ITC.Actions.size();
  ITC.Actions.push_back(
      {InteractMsg, [Entity, InteractIdx](auto &EHC, auto Et, auto &Reg) {
         auto &DC = Reg.template get<DoorComp>(Entity);
         if (DC.isLocked()) {
           if (unlockDoor(Reg, Entity, Et, InteractIdx)) {
             EHC.publish(PlayerInfoMessageEvent() << "You unlock the door.");
           } else {
             EHC.publish(PlayerInfoMessageEvent()
                         << "You fail to unlock the door.");
           }
         } else if (DC.IsOpen) {
           closeDoor(Reg, Entity, InteractIdx);
           EHC.publish(PlayerInfoMessageEvent() << "You open the door.");
         } else {
           openDoor(Reg, Entity, InteractIdx);
           EHC.publish(PlayerInfoMessageEvent() << "You close the door.");
         }
         (void)Et;
       }});

  auto &TC = Reg.get_or_emplace<TileComp>(Entity);
  if (IsOpen) {
    TC.T = OpenTile;
    TC.ZIndex = -2;
  } else {
    TC.T = ClosedTile;
    TC.ZIndex = 0;
  }
}

LootedInteractCompAssembler::LootedInteractCompAssembler(
    bool IsLooted, bool IsPersistent, Tile DefaultTile, Tile LootedTile,
    const std::string &InteractText, const std::string &LootName)
    : IsLooted(IsLooted), IsPersistent(IsPersistent), DefaultTile(DefaultTile),
      LootedTile(LootedTile), InteractText(InteractText), LootName(LootName) {}

namespace {
void handleLoot(entt::registry &Reg, const entt::entity &Entity,
                const entt::entity &ActEt, EventHubConnector &EHC) {
  auto &TC = Reg.template get<TileComp>(Entity);
  auto &LIC = Reg.template get<LootInteractComp>(Entity);
  if (!LIC.IsLooted) {
    TC.T = LIC.LootedTile;
    LIC.IsLooted = true;
  }
  EHC.publish(LootEvent{{}, LIC.LootName, ActEt, Entity, &Reg});
}

} // namespace

void LootedInteractCompAssembler::assemble(entt::registry &Reg,
                                           entt::entity Entity) const {

  auto &ITC = Reg.get_or_emplace<InteractableComp>(Entity);
  ITC.Actions.push_back({InteractText, [Entity](auto &EHC, auto Et, auto &Reg) {
                           handleLoot(Reg, Entity, Et, EHC);
                         }});
  Reg.emplace<LootInteractComp>(Entity, IsLooted, IsPersistent, DefaultTile,
                                LootedTile, LootName);

  auto &TC = Reg.get_or_emplace<TileComp>(Entity);
  if (IsLooted) {
    TC.T = LootedTile;
  } else {
    TC.T = DefaultTile;
  }
}

WorldEntryInteractableCompAssembler::WorldEntryInteractableCompAssembler(
    const std::string &LevelName)
    : LevelName(LevelName) {}

void WorldEntryInteractableCompAssembler::assemble(entt::registry &Reg,
                                                   entt::entity Entity) const {
  auto &ITC = Reg.get_or_emplace<InteractableComp>(Entity);
  ITC.Actions.push_back(
      {"Enter Dungeon", [this, Entity](auto &EHC, auto SrcEt, auto &) {
         EHC.publish(SwitchGameWorldEvent{{}, LevelName, SrcEt, Entity});
       }});
}

LevelEntryExitAssembler::LevelEntryExitAssembler(bool IsExit, int LevelId)
    : IsExit(IsExit), LevelId(LevelId) {}

void LevelEntryExitAssembler::assemble(entt::registry &Reg,
                                       entt::entity Entity) const {
  const char *InteractText = IsExit ? "Exit level" : "Enter level";
  auto &ITC = Reg.get_or_emplace<InteractableComp>(Entity);
  ITC.Actions.push_back(
      {InteractText, [this, Entity](auto &EHC, auto SrcEt, auto &Reg) {
         auto LId = -1;
         if (IsExit) {
           LId = Reg.template get<LevelStartComp>(Entity).NextLevelId;
         } else {
           LId = Reg.template get<LevelEndComp>(Entity).NextLevelId;
         }
         EHC.publish(
             SwitchLevelEvent{{}, LId, /*ToEntry=*/!IsExit, SrcEt, Entity});
       }});
  if (IsExit) {
    Reg.emplace<LevelStartComp>(Entity, LevelId);
  } else {
    Reg.emplace<LevelEndComp>(Entity, LevelId);
  }
}

void HealerInteractableCompAssembler::assemble(entt::registry &Reg,
                                               entt::entity Entity) const {
  auto &ITC = Reg.get_or_emplace<InteractableComp>(Entity);
  ITC.Actions.push_back({"Heal", [](auto &EHC, auto SrcEt, auto &Reg) {
                           auto &HC = Reg.template get<HealthComp>(SrcEt);
                           HC.Value = HC.MaxValue;
                           EHC.publish(PlayerInfoMessageEvent()
                                       << "You feel better.");
                         }});
}

void ShopAssembler::assemble(entt::registry &Reg, entt::entity Entity) const {
  auto &ITC = Reg.get_or_emplace<InteractableComp>(Entity);
  ITC.Actions.push_back({"Shop", [](auto &EHC, auto SrcEt, auto &Reg) {
                           EHC.publish(PlayerInfoMessageEvent()
                                       << "Shop not implemented yet.");
                           (void)SrcEt;
                           (void)Reg;
                           // EHC.publish(ShopOpenEvent{{}, Et, Entity,
                           // &Reg});
                         }});
}

void WorkbenchAssembler::assemble(entt::registry &Reg,
                                  entt::entity Entity) const {
  auto &ITC = Reg.get_or_emplace<InteractableComp>(Entity);
  ITC.Actions.push_back(
      {"Craft", [Entity](auto &EHC, auto SrcEt, auto &Reg) {
         (void)Reg;
         auto &Crafter = Reg.ctx().template get<GameContext>().Crafter;
         InventoryHandler InvHandler(Entity, Reg, Crafter);
         InvHandler.setEventHub(EHC.getEventHub());
         if (InvHandler.tryCraftItems(SrcEt)) {
           EHC.publish(PlayerInfoMessageEvent() << "Crafting successful");
         } else {
           EHC.publish(PlayerInfoMessageEvent() << "Crafting failed");
         }
       }});
  ITC.Actions.push_back({"Known Recipes", [](auto &EHC, auto SrcEt, auto &Reg) {
                           CraftEvent CE;
                           CE.Entity = SrcEt;
                           CE.Registry = &Reg;
                           EHC.publish(CE);
                         }});
}

bool WorkbenchAssembler::isPostProcess() const { return true; }

StatsCompAssembler::StatsCompAssembler(StatPoints Stats) : Stats(Stats) {}

void StatsCompAssembler::assemble(entt::registry &Reg,
                                  entt::entity Entity) const {
  Reg.emplace<StatsComp>(Entity).Base = Stats;
}

} // namespace rogue