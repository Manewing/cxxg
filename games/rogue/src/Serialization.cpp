#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>
#include <rogue/Components/Serialization.h>
#include <rogue/Context.h>
#include <rogue/ItemDatabase.h>
#include <rogue/ItemEffectImpl.h>
#include <rogue/Level.h>
#include <rogue/Serialization.h>

#define CEREAL_REGISTER_ITEM_EFFECT_TYPE(Type)                                 \
  CEREAL_REGISTER_TYPE(Type);                                                  \
  CEREAL_REGISTER_POLYMORPHIC_RELATION(rogue::ItemEffect, Type);

CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::NullEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::HealItemEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::DamageItemEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::DismantleEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::PoisonDebuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::BleedingDebuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::StatsBuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::ArmorBuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::BlockBuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::HealthRegenBuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::ManaRegenBuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::BlindedDebuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::MindVisionBuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::InvisibilityBuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::StatsTimedBuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::StatsBuffPerHitEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::CoHTargetBleedingDebuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::CoHTargetPoisonDebuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::CoHTargetBlindedDebuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::LifeStealBuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::SetMeleeCompEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::SetRangedCompEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::RemovePoisonEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::RemoveBleedingEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::RemoveBlindedEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::RemoveHealthRegenEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::RemoveManaRegenEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::RemovePoisonDebuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::RemoveBleedingDebuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::RemoveBlindedDebuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::RemoveHealthRegenBuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::RemoveManaRegenBuffEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::ManaItemEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::SweepingStrikeEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::SmiteEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::DiscAreaHitEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::LearnRecipeEffect);
CEREAL_REGISTER_ITEM_EFFECT_TYPE(rogue::SpawnEntityEffect);

namespace rogue::serialize {

ItemInfo ItemInfo::createFrom(const Item &It) {
  return {It.getId(), It.StackSize, It.getSpecialization(),
          It.doesSpecOverride()};
}

Item ItemInfo::create(const ItemDatabase &ItemDb) const {
  auto &Proto = ItemDb.getItemProto(Id);
  return Item(Proto, StackSize, Specialization, SpecOverrides);
}

InventoryInfo InventoryInfo::createFrom(const InventoryComp &IC) {
  InventoryInfo II;
  II.Items.reserve(IC.Inv.getItems().size());
  for (const auto &It : IC.Inv.getItems()) {
    II.Items.push_back(ItemInfo::createFrom(It));
  }
  return II;
}

void InventoryInfo::applyTo(const ItemDatabase &ItemDb,
                            InventoryComp &IC) const {
  IC.Inv.clear();
  for (const auto &Info : Items) {
    IC.Inv.addItem(Info.create(ItemDb));
  }
}

EquipmentInfo EquipmentInfo::createFrom(const EquipmentComp &EC) {
  EquipmentInfo EI;
  EI.Slots.reserve(EC.Equip.all().size());
  for (const auto &Slot : EC.Equip.all()) {
    if (Slot->It) {
      EI.Slots.push_back(ItemInfo::createFrom(*Slot->It));
    }
  }
  return EI;
}

void EquipmentInfo::applyTo(const ItemDatabase &ItemDb, EquipmentComp &EC,
                            entt::entity Et, entt::registry &Reg) const {
  for (const auto &SlotInfo : Slots) {
    EC.Equip.equip(SlotInfo.create(ItemDb), Et, Reg);
  }
}

SaveGameSerializer SaveGameSerializer::loadFromFile(const SaveGameInfo &SGI) {
  std::ifstream InFile(SGI.Path);
  if (!InFile) {
    throw std::runtime_error("SaveGame: Failed to open file: " +
                             SGI.Path.string());
  }

  SaveGameSerializer SaveGame;

  if (SGI.JSON) {
    cereal::JSONInputArchive Ar(InFile);
    Ar(SaveGame);
  } else {
    cereal::BinaryInputArchive Ar(InFile);
    Ar(SaveGame);
  }

  return SaveGame;
}

SaveGameSerializer SaveGameSerializer::create(Level &Lvl) {
  SaveGameSerializer SaveGame;
  Lvl.Reg.view<const IdComp, const DoorComp>().each(
      [&SaveGame](const auto &IdC, const auto &DC) {
        SaveGame.Doors[IdC.Id] = DC;
      });

  Lvl.Reg.view<const IdComp, const EquipmentComp>().each(
      [&SaveGame](const auto &IdC, const auto &EC) {
        SaveGame.EquipmentInfos[IdC.Id] = EquipmentInfo::createFrom(EC);
      });

  Lvl.Reg.view<const IdComp, const InventoryComp>().each(
      [&SaveGame](const auto &IdC, const auto &IC) {
        SaveGame.InventoryInfos[IdC.Id] = InventoryInfo::createFrom(IC);
      });

  Lvl.Reg.view<const IdComp, const PlayerComp>().each(
      [&SaveGame](const auto &IdC, const auto &PC) {
        SaveGame.PlayerInfos[IdC.Id].KnownRecipes = PC.KnownRecipes;
      });

  return SaveGame;
}

void SaveGameSerializer::apply(Level &Lvl) {
  auto &ItemDb = Lvl.Reg.ctx().get<GameContext>().ItemDb;

  Lvl.Reg.view<const IdComp, DoorComp>().each(
      [this, &Lvl](auto Entity, const auto &IdC, auto &DC) {
        auto It = Doors.find(IdC.Id);
        if (It == Doors.end()) {
          throw std::runtime_error("SaveGame: Door not found: Id=" +
                                   std::to_string(IdC.Id));
        }
        DC = It->second;

        if (DC.IsOpen) {
          DC.openDoor(Lvl.Reg, Entity);
        } else {
          DC.closeDoor(Lvl.Reg, Entity);
        }
      });

  Lvl.Reg.view<const IdComp, EquipmentComp>().each(
      [this, &ItemDb, &Lvl](auto Entity, const auto &IdC, auto &EC) {
        auto It = EquipmentInfos.find(IdC.Id);
        if (It == EquipmentInfos.end()) {
          throw std::runtime_error("SaveGame: Equipment not found in save for Id=" +
                                   std::to_string(IdC.Id));
        }
        It->second.applyTo(ItemDb, EC, Entity, Lvl.Reg);
      });

  Lvl.Reg.view<const IdComp, InventoryComp>().each(
      [this, &ItemDb](auto &IdC, auto &IC) {
        auto It = InventoryInfos.find(IdC.Id);
        if (It == InventoryInfos.end()) {
          throw std::runtime_error("SaveGame: Inventory not found in save for Id=" +
                                   std::to_string(IdC.Id));
        }
        It->second.applyTo(ItemDb, IC);
      });

  Lvl.Reg.view<const IdComp, PlayerComp>().each([this](auto &IdC, auto &PC) {
    auto It = PlayerInfos.find(IdC.Id);
    if (It == PlayerInfos.end()) {
      throw std::runtime_error("SaveGame: Player not found for Id=" +
                               std::to_string(IdC.Id));
    }
    PC.KnownRecipes = It->second.KnownRecipes;
  });
}

void SaveGameSerializer::saveToFile(const SaveGameInfo &SGI) const {
  std::ofstream OutFile(SGI.Path);
  if (!OutFile) {
    throw std::runtime_error("SaveGame: Failed to open file: " +
                             SGI.Path.string());
  }

  if (SGI.JSON) {
    cereal::JSONOutputArchive Ar(OutFile);
    Ar(*this);
  } else {
    cereal::BinaryOutputArchive Ar(OutFile);
    Ar(*this);
  }
}

} // namespace rogue::serialize