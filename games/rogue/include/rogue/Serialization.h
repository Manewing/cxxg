#ifndef ROGUE_SERIALIZATION_H
#define ROGUE_SERIALIZATION_H

#include <filesystem>
#include <rogue/Components/Helpers.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/Level.h>
#include <rogue/Components/Player.h>
#include <rogue/SaveGame.h>

namespace rogue {
class Level;
class ItemDatabase;
} // namespace rogue

namespace rogue::serialize {

struct ItemInfo {
  static ItemInfo createFrom(const Item &It);
  Item create(const ItemDatabase &ItemDb) const;

  int Id = 0;
  int StackSize = 1;
  std::shared_ptr<const ItemPrototype> Specialization = nullptr;
  bool SpecOverrides = false;

  template <class Archive> void serialize(Archive &Ar) {
    Ar(Id, StackSize, Specialization, SpecOverrides);
  }
};

struct InventoryInfo {
  static InventoryInfo createFrom(const InventoryComp &IC);
  void applyTo(const ItemDatabase &ItemDb, InventoryComp &IC) const;

  std::vector<ItemInfo> Items;

  template <class Archive> void serialize(Archive &Ar) { Ar(Items); }
};

struct EquipmentInfo {
  static EquipmentInfo createFrom(const EquipmentComp &EC);
  void applyTo(const ItemDatabase &ItemDb, EquipmentComp &EC, entt::entity Et,
               entt::registry &Reg) const;

  std::vector<ItemInfo> Slots;

  template <class Archive> void serialize(Archive &Ar) { Ar(Slots); }
};

struct PlayerInfo {
  std::set<CraftingRecipeId> KnownRecipes;

  template <class Archive> void serialize(Archive &Ar) { Ar(KnownRecipes); }
};

class SaveGameSerializer {
public:
  static SaveGameSerializer loadFromFile(const SaveGameInfo &SGI);

  static SaveGameSerializer create(Level &Lvl);

public:
  void apply(Level &Lvl);
  void saveToFile(const SaveGameInfo &SGI) const;

  template <class Archive> void load(Archive &Ar) {
    Ar(Doors, EquipmentInfos, InventoryInfos, PlayerInfos);
  }

  template <class Archive> void save(Archive &Ar) const {
    Ar(Doors, EquipmentInfos, InventoryInfos, PlayerInfos);
  }

private:
  std::map<std::size_t, DoorComp> Doors;
  std::map<std::size_t, EquipmentInfo> EquipmentInfos;
  std::map<std::size_t, InventoryInfo> InventoryInfos;
  std::map<std::size_t, PlayerInfo> PlayerInfos;
};

} // namespace rogue::serialize

#endif // #ifndef ROGUE_SERIALIZATION_H