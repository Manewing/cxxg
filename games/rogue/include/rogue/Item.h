#ifndef ROGUE_ITEM_H
#define ROGUE_ITEM_H

#include <entt/entt.hpp>
#include <memory>
#include <optional>
#include <rogue/Components/Stats.h>
#include <string>
#include <vector>
#include <ymir/Enum.hpp>

namespace rogue {

enum class ItemType {
  None = 0x0,

  // Equipment types
  Ring = 0x1,
  Amulet = 0x2,
  Helmet = 0x4,
  ChestPlate = 0x8,
  Pants = 0x10,
  Boots = 0x20,
  Weapon = 0x40,
  Shield = 0x80,
  EquipmentMask = 0xff,

  // general item types
  Generic = 0x100,
  Consumable = 0x200,
  Quest = 0x400,
  Crafting = 0x800,
  GeneralMask = 0xf00,

  // mask for all items
  AnyMask = 0xfffffff
};

}

YMIR_BITFIELD_ENUM(rogue::ItemType);

namespace rogue {

class UseInterface {
public:
  virtual bool canUseOn(const entt::entity &Entity,
                        entt::registry &Reg) const = 0;
  virtual void useOn(const entt::entity &Entity, entt::registry &Reg) const = 0;
};

class EquipInterface {
public:
  virtual bool canEquipOn(const entt::entity &Entity,
                          entt::registry &Reg) const = 0;
  virtual void equipOn(const entt::entity &Entity,
                       entt::registry &Reg) const = 0;
  virtual bool canUnequipFrom(const entt::entity &Entity,
                              entt::registry &Reg) const = 0;
  virtual void unequipFrom(const entt::entity &Entity,
                           entt::registry &Reg) const = 0;
};

// Add durability to items:
//  Durability
//  MaxDurability
// Both get reduced by being used, MaxDurability way slower than durability
// If durability is zero item breaks
// Durability can be restored to MaxDurability by repairing
// MaxDurability can only be restored with rare special items

class ItemEffect : public UseInterface, public EquipInterface {
public:
  virtual ~ItemEffect() = default;

  bool canUseOn(const entt::entity &, entt::registry &) const override {
    return false;
  }

  void useOn(const entt::entity &, entt::registry &) const override {}

  bool canEquipOn(const entt::entity &, entt::registry &) const override {
    // Generally handled by Item slot, only to veto by effect
    return true;
  }

  void equipOn(const entt::entity &, entt::registry &) const override {}

  bool canUnequipFrom(const entt::entity &, entt::registry &) const override {
    // Generally handled by Item slot, only to veto by effect
    return true;
  }

  void unequipFrom(const entt::entity &, entt::registry &) const override {}
};

class HealItemEffect : public ItemEffect {
public:
  HealItemEffect(StatValue Amount);
  bool canUseOn(const entt::entity &Et, entt::registry &Reg) const final;
  void useOn(const entt::entity &Et, entt::registry &Reg) const final;

private:
  StatValue Amount;
};

class DamageItemEffect : public ItemEffect {
public:
  DamageItemEffect(StatValue Amount);
  bool canUseOn(const entt::entity &Et, entt::registry &Reg) const final;
  void useOn(const entt::entity &Et, entt::registry &Reg) const final;

private:
  StatValue Amount;
};

template <typename BuffType, typename... RequiredComps>
class ApplyBuffEffect : public ItemEffect {
public:
  explicit ApplyBuffEffect(const BuffType &Buff) : Buff(Buff) {}

  bool canUseOn(const entt::entity &Et, entt::registry &Reg) const final {
    return Reg.all_of<RequiredComps...>(Et);
  }

  void useOn(const entt::entity &Et, entt::registry &Reg) const final {
    auto ExistingBuff = Reg.try_get<BuffType>(Et);
    if (ExistingBuff) {
      ExistingBuff->add(Buff);
    }
    Reg.emplace<BuffType>(Et);
  }

private:
  BuffType Buff;
};

class ItemPrototype : public UseInterface, public EquipInterface {
public:
  ItemPrototype(int ItemId, std::string Name, ItemType Type, int MaxStatckSize,
                std::vector<std::shared_ptr<ItemEffect>> Effects);

  // add:
  // attack melee
  // attack ranged
  // defnese
  // craft
  bool canUseOn(const entt::entity &Entity, entt::registry &Reg) const final;
  void useOn(const entt::entity &Entity, entt::registry &Reg) const final;

  bool canEquipOn(const entt::entity &Entity, entt::registry &Reg) const final;
  void equipOn(const entt::entity &Entity, entt::registry &Reg) const final;

  bool canUnequipFrom(const entt::entity &Entity,
                      entt::registry &Reg) const final;
  void unequipFrom(const entt::entity &Entity, entt::registry &Reg) const final;

public:
  int ItemId;
  std::string Name;
  ItemType Type;
  int MaxStatckSize = 1;

  std::vector<std::shared_ptr<ItemEffect>> Effects;
};

class Item : public UseInterface, public EquipInterface {
public:
  Item(const ItemPrototype &Proto, int StackSize = 1,
       const std::shared_ptr<ItemPrototype> &Specialization = nullptr);
  virtual ~Item() = default;

  std::string getName() const;

  ItemType getType() const;

  /// Returns true if other Item has same prototype and specialization
  bool isSameKind(const Item &Other) const;

  bool canUseOn(const entt::entity &Entity, entt::registry &Reg) const final;
  void useOn(const entt::entity &Entity, entt::registry &Reg) const final;

  bool canEquipOn(const entt::entity &Entity, entt::registry &Reg) const final;
  void equipOn(const entt::entity &Entity, entt::registry &Reg) const final;

  bool canUnequipFrom(const entt::entity &Entity,
                      entt::registry &Reg) const final;
  void unequipFrom(const entt::entity &Entity, entt::registry &Reg) const final;

public:
  int StackSize = 1;

private:
  const ItemPrototype &getProto() const;

private:
  const ItemPrototype *Proto = nullptr;
  std::shared_ptr<const ItemPrototype> Specialization = nullptr;
};

class Equipment {
public:
  std::optional<Item> Ring;
  std::optional<Item> Amulet;
  std::optional<Item> Helmet;
  std::optional<Item> ChestPlat;
  std::optional<Item> Pants;
  std::optional<Item> Boots;
  std::optional<Item> Weapon;
  std::optional<Item> OffHand;

  bool canEquip(ItemType Type) const;
  bool equip(const Item &Item);
  Item unequip(ItemType Type);
};

} // namespace rogue

#endif // #ifndef ROGUE_ITEM_H