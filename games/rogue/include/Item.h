#ifndef ROGUE_ITEM_H
#define ROGUE_ITEM_H

#include <string>

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

class ItemPrototype {
public:
  ItemPrototype(int ItemId, std::string Name, ItemType Type,
                int MaxStatckSize = 1);

  int ItemId;
  std::string Name;
  ItemType Type;
  int MaxStatckSize = 1;
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

#endif // #ifndef ROGUE_ITEM_H