#include <rogue/Components/Serialization.h>
#include <rogue/Context.h>
#include <rogue/CraftingHandler.h>
#include <rogue/EntityAssemblers.h>
#include <rogue/Event.h>
#include <rogue/InventoryHandler.h>
#include <rogue/ItemDatabase.h>
#include <rogue/ItemEffectImpl.h>

namespace rogue {

TileCompAssembler::TileCompAssembler(Tile T) : T(T) {}

void TileCompAssembler::assemble(entt::registry &Reg,
                                 entt::entity Entity) const {
  // There are some assemblers that already add a tile component, sanity check
  // that there is none already (indicates incorrect usage of assemblers)
  if (Reg.all_of<TileComp>(Entity)) {
    throw std::runtime_error("Entity already has a tile component");
  }
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

LineOfSightCompAssembler::LineOfSightCompAssembler(unsigned Range)
    : Range(Range) {}

void LineOfSightCompAssembler::assemble(entt::registry &Reg,
                                        entt::entity Entity) const {
  Reg.emplace<LineOfSightComp>(Entity, Range, Range);
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
  DC.ActionIdx = ITC.Actions.size();
  ITC.Actions.push_back(
      {InteractMsg, [Entity](auto &EHC, auto Et, auto &Reg) {
         auto &DC = Reg.template get<DoorComp>(Entity);
         if (DC.isLocked()) {
           if (DoorComp::unlockDoor(Reg, Entity, Et)) {
             EHC.publish(PlayerInfoMessageEvent() << "You unlock the door.");
           } else {
             EHC.publish(PlayerInfoMessageEvent()
                         << "You fail to unlock the door.");
           }
         } else if (DC.IsOpen) {
           DoorComp::closeDoor(Reg, Entity);
           EHC.publish(PlayerInfoMessageEvent() << "You open the door.");
         } else {
           DoorComp::openDoor(Reg, Entity);
           EHC.publish(PlayerInfoMessageEvent() << "You close the door.");
         }
         (void)Et;
       }});

  auto &TC = Reg.get_or_emplace<TileComp>(Entity);
  if (IsOpen) {
    TC.T = OpenTile;
  } else {
    TC.T = ClosedTile;
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
  ITC.Actions.push_back({"Craft", [Entity](auto &EHC, auto SrcEt, auto &Reg) {
                           (void)Reg;
                           auto &Crafter =
                               Reg.ctx().template get<GameContext>().Crafter;
                           InventoryHandler InvHandler(Entity, Reg, Crafter);
                           InvHandler.setEventHub(EHC.getEventHub());
                           InvHandler.tryCraftItems(SrcEt);
                         }});
  ITC.Actions.push_back({"Known Recipes", [](auto &EHC, auto SrcEt, auto &Reg) {
                           CraftEvent CE;
                           CE.Entity = SrcEt;
                           CE.Registry = &Reg;
                           EHC.publish(CE);
                         }});
}

SpawnEntityPostInteractionAssembler::SpawnEntityPostInteractionAssembler(
    const std::string &EntityName, double Chance, unsigned Uses)
    : EntityName(EntityName), Chance(Chance), Uses(Uses) {}

void SpawnEntityPostInteractionAssembler::assemble(entt::registry &Reg,
                                                   entt::entity Entity) const {
  auto &ITC = Reg.get_or_emplace<InteractableComp>(Entity);
  if (Uses == 0) {
    ITC.PreActionExecuteFns.push_back(
        [this](auto &, auto SrcEt, auto &Reg) mutable {
          SpawnEntityEffect SEE(EntityName, Chance);
          SEE.applyTo(SrcEt, SrcEt, Reg);
        });
    return;
  }
  ITC.PreActionExecuteFns.push_back(
      [this, Uses = Uses](auto &, auto SrcEt, auto &Reg) mutable {
        if (Uses == 0) {
          return;
        }
        Uses--;
        SpawnEntityEffect SEE(EntityName, Chance);
        SEE.applyTo(SrcEt, SrcEt, Reg);
      });
}

bool WorkbenchAssembler::isPostProcess() const { return true; }

StatsCompAssembler::StatsCompAssembler(StatPoints Stats) : Stats(Stats) {}

void StatsCompAssembler::assemble(entt::registry &Reg,
                                  entt::entity Entity) const {
  Reg.emplace<StatsComp>(Entity).Base = Stats;
}

DamageCompAssembler::DamageCompAssembler(const DamageComp &DC) : DC(DC) {}

void DamageCompAssembler::assemble(entt::registry &Reg,
                                   entt::entity Entity) const {
  auto NewDC = DC;
  NewDC.Source = Entity;
  Reg.emplace<DamageComp>(Entity, NewDC);
}

EffectExecutorCompAssembler::EffectExecutorCompAssembler(
    EffectExecutorComp Executer)
    : Executer(std::move(Executer)) {}

void EffectExecutorCompAssembler::assemble(entt::registry &Reg,
                                           entt::entity Entity) const {
  Reg.emplace<EffectExecutorComp>(Entity, Executer);
}

SerializationIdCompAssembler::SerializationIdCompAssembler(std::size_t Id)
    : Id(Id) {}

void SerializationIdCompAssembler::assemble(entt::registry &Reg,
                                            entt::entity Entity) const {
  Reg.get_or_emplace<serialize::IdComp>(Entity).Id = Id;
}

} // namespace rogue