#ifndef ROGUE_ITEM_TYPE_H
#define ROGUE_ITEM_TYPE_H

#include <iosfwd>
#include <optional>
#include <rogue/BitOps.h>
#include <string>
#include <ymir/Enum.hpp>

namespace rogue {

class ItemType {
public:
  enum ValueType : int {
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
    Ranged = 0x100,
    EquipmentMask = 0xfff,

    /// Items that can be used as a basis for crafting a new item
    CraftingBase = 0x1000,

    /// Consumable items can be used to apply an effect to an entity
    Consumable = 0x2000,

    Quest = 0x4000,
    Crafting = 0x8000,
    GeneralMask = 0xf000,

    // mask for all items
    AnyMask = 0xfffffff
  };
  using value_type = ValueType;

public:
  static std::optional<ItemType> parseString(const std::string &Str);
  static ItemType fromString(const std::string &Str);

public:
  ItemType() = default;
  explicit ItemType(int Value) : Value(static_cast<ValueType>(Value)) {}
  ItemType(ValueType Value) : Value(Value) {}
  ItemType(const ItemType &) = default;
  ItemType(ItemType &&) = default;
  ItemType &operator=(const ItemType &) = default;
  ItemType &operator=(ItemType &&) = default;

  operator ValueType() const { return Value; }
  explicit operator bool() const { return Value != None; }

  bool operator==(const ItemType &Other) const { return Value == Other.Value; }
  bool operator!=(const ItemType &Other) const { return Value != Other.Value; }

  bool operator==(ValueType Other) const { return Value == Other; }
  bool operator!=(ValueType Other) const { return Value != Other; }

  /// Returns true if all flags set in other are also set in this
  bool is(ItemType Other) const;

  std::string str() const;

private:
  ValueType Value = None;
};

std::ostream &operator<<(std::ostream &Out, const ItemType &Type);

class CapabilityFlags {
public:
  enum ValueType : int {
    None = 0x0,
    UseOn = 0x1,
    EquipOn = 0x2,
    UnequipFrom = 0x4,

    // Mask all equipment
    Equipment = 0x6,

    // Dismantle item
    Dismantle = 0x8,

    // Ranged, indicates that the item can be used from a distance on an entity
    // different than the entity using the item
    Ranged = 0x10,

    // Adjacent, indicates that the item can be used from an adjacent tile on an
    // entity different than the entity using the item
    Adjacent = 0x20,

    // Self, indicates that the item can be used on the entity using the item
    Self = 0x40,

    // Skill, indicates that the item can be used to cast a skill
    Skill = 0x80,
  };
  using value_type = ValueType;

public:
  static std::optional<CapabilityFlags> parseString(const std::string &Str);
  static CapabilityFlags fromString(const std::string &Str);

public:
  CapabilityFlags() = default;
  explicit CapabilityFlags(int Value) : Value(static_cast<ValueType>(Value)) {}
  CapabilityFlags(ValueType Value) : Value(Value) {}
  CapabilityFlags(const CapabilityFlags &) = default;
  CapabilityFlags(CapabilityFlags &&) = default;
  CapabilityFlags &operator=(const CapabilityFlags &) = default;
  CapabilityFlags &operator=(CapabilityFlags &&) = default;

  operator ValueType() const { return Value; }
  explicit operator bool() const;

  bool operator==(const CapabilityFlags &Other) const {
    return Value == Other.Value;
  }
  bool operator!=(const CapabilityFlags &Other) const {
    return Value != Other.Value;
  }

  /// Checks if both adjacent and \p Other flags are set
  bool isAdjacent(CapabilityFlags Other) const;

  /// Checks if both ranged and \p Other flags are set
  bool isRanged(CapabilityFlags Other) const;

  bool operator==(ValueType Other) const { return Value == Other; }
  bool operator!=(ValueType Other) const { return Value != Other; }

  const char *str() const;

private:
  ValueType Value = None;
};

std::ostream &operator<<(std::ostream &Out, const CapabilityFlags &Flags);

} // namespace rogue

YMIR_BITFIELD_ENUM(rogue::ItemType::ValueType);
ROGUE_BIT_OPS_TYPE(rogue::ItemType);

YMIR_BITFIELD_ENUM(rogue::CapabilityFlags::ValueType);
ROGUE_BIT_OPS_TYPE(rogue::CapabilityFlags);

#endif // #ifndef ROGUE_ITEM_TYPE_H