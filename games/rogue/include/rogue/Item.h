#ifndef ROGUE_ITEM_H
#define ROGUE_ITEM_H

#include <entt/entt.hpp>
#include <memory>
#include <string>
#include <vector>

#include <rogue/Components/Stats.h>

namespace rogue {

enum ItemType {
  // Equipment types
  RING = 0x1,
  AMULET = 0x2,
  HELMET = 0x4,
  CHEST_PLATE = 0x8,
  PANTS = 0x10,
  BOOTS = 0x20,
  WEAPON = 0x40,
  SHIELD = 0x80,
  EQUIPMENT_MASK = 0xff,

  // General item types
  GENERIC = 0x100,
  CONSUMABLE = 0x200,
  QUEST = 0x400,
  CRAFTING = 0x800,
  GENERAL_MASK = 0xf00,

  // Mask for all items
  ANY_MASK = 0xffffffff
};

// Add durability to items:
//  Durability
//  MaxDurability
// Both get reduced by being used, MaxDurability way slower than durability
// If durability is zero item breaks
// Durability can be restored to MaxDurability by repairing
// MaxDurability can only be restored with rare special items

class ItemEffect {
public:
  virtual bool canUseOn(const entt::entity &Et, entt::registry &Reg) const = 0;
  virtual int useOn(const entt::entity &Et, entt::registry &Reg,
                    int Num) const = 0;
  virtual ~ItemEffect() = default;
};
class HealItemEffect : public ItemEffect {
public:
  HealItemEffect(StatValue Amount);
  bool canUseOn(const entt::entity &Et, entt::registry &Reg) const final;
  int useOn(const entt::entity &Et, entt::registry &Reg, int Num) const final;

private:
  StatValue Amount;
};
class DamageItemEffect : public ItemEffect {
public:
  DamageItemEffect(StatValue Amount);
  bool canUseOn(const entt::entity &Et, entt::registry &Reg) const final;
  int useOn(const entt::entity &Et, entt::registry &Reg, int Num) const final;

private:
  StatValue Amount;
};

class ItemPrototype {
public:
  ItemPrototype(int ItemId, std::string Name, ItemType Type, int MaxStatckSize,
                std::vector<std::shared_ptr<ItemEffect>> Effects);

  // add:
  // attack melee
  // attack ranged
  // defnese
  // craft
  bool canUseOn(const entt::entity &Entity, entt::registry &Reg) const;
  void useOn(const entt::entity &Entity, entt::registry &Reg, int Item) const;

public:
  int ItemId;
  std::string Name;
  ItemType Type;
  int MaxStatckSize = 1;
  std::vector<std::shared_ptr<ItemEffect>> Effects;
};

class Item {
public:
  Item(const ItemPrototype &Proto, int StackSize = 1);

  const ItemPrototype &getProto() const;

public:
  int StackSize = 1;

protected:
  const ItemPrototype *Proto = nullptr;
};

}

#endif // #ifndef ROGUE_ITEM_H