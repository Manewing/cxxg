#ifndef ROGUE_ITEM_H
#define ROGUE_ITEM_H

#include <array>
#include <entt/entt.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Stats.h>
#include <rogue/ItemType.h>
#include <string>
#include <vector>

namespace rogue {

// Add durability to items:
//  Durability
//  MaxDurability
// Both get reduced by being used, MaxDurability way slower than durability
// If durability is zero item breaks
// Durability can be restored to MaxDurability by repairing
// MaxDurability can only be restored with rare special items

class ItemEffect {
public:
  virtual ~ItemEffect() = default;

  virtual bool canApplyTo(const entt::entity &, entt::registry &) const {
    return true;
  }

  virtual void applyTo(const entt::entity &, entt::registry &) const {}

  virtual bool canRemoveFrom(const entt::entity &, entt::registry &) const {
    return true;
  }

  virtual void removeFrom(const entt::entity &, entt::registry &) const {}
};

class HealItemEffect : public ItemEffect {
public:
  explicit HealItemEffect(StatValue Amount);
  StatValue getAmount() const { return Amount; }

  bool canApplyTo(const entt::entity &Et, entt::registry &Reg) const final;
  void applyTo(const entt::entity &Et, entt::registry &Reg) const final;

private:
  StatValue Amount;
};

class DamageItemEffect : public ItemEffect {
public:
  explicit DamageItemEffect(StatValue Amount);
  StatValue getAmount() const { return Amount; }

  bool canApplyTo(const entt::entity &Et, entt::registry &Reg) const final;
  void applyTo(const entt::entity &Et, entt::registry &Reg) const final;

private:
  StatValue Amount;
};

class ApplyBuffItemEffectBase : public ItemEffect {
public:
  virtual const BuffBase &getBuff() const = 0;
};

template <typename BuffType, typename... RequiredComps>
class ApplyBuffItemEffect : public ApplyBuffItemEffectBase {
public:
  explicit ApplyBuffItemEffect(const BuffType &Buff) : Buff(Buff) {}

  const BuffBase &getBuff() const final { return Buff; }

  bool canApplyTo(const entt::entity &Et, entt::registry &Reg) const final {
    return Reg.all_of<RequiredComps...>(Et);
  }

  void applyTo(const entt::entity &Et, entt::registry &Reg) const final {
    auto ExistingBuff = Reg.try_get<BuffType>(Et);
    if (ExistingBuff) {
      ExistingBuff->add(Buff);
    } else {
      Reg.emplace<BuffType>(Et, Buff);
    }
  }

  bool canRemoveFrom(const entt::entity &Et, entt::registry &Reg) const final {
    return Reg.all_of<RequiredComps...>(Et);
  }

  void removeFrom(const entt::entity &Et, entt::registry &Reg) const final {
    auto ExistingBuff = Reg.try_get<BuffType>(Et);
    if (ExistingBuff) {
      if (ExistingBuff->remove(Buff)) {
        Reg.erase<BuffType>(Et);
      }
    }
  }

private:
  BuffType Buff;
};

class ItemPrototype {
public:
  struct EffectInfo {
    CapabilityFlags Flags;
    std::shared_ptr<ItemEffect> Effect;
  };

public:
  static bool canApply(ItemType Type, CapabilityFlags Flags);

public:
  ItemPrototype(int ItemId, std::string Name, std::string Description,
                ItemType Type, int MaxStackSize,
                std::vector<EffectInfo> Effects);

  // TODO add:
  // attack melee
  // attack ranged
  // defense
  // craft

  bool canApplyTo(const entt::entity &Entity, entt::registry &Reg,
                  CapabilityFlags Flags) const;
  void applyTo(const entt::entity &Entity, entt::registry &Reg,
               CapabilityFlags Flags) const;
  bool canRemoveFrom(const entt::entity &Entity, entt::registry &Reg,
                     CapabilityFlags Flags) const;
  void removeFrom(const entt::entity &Entity, entt::registry &Reg,
                  CapabilityFlags Flags) const;

public:
  int ItemId;
  std::string Name;
  std::string Description;
  ItemType Type = ItemType::None;
  int MaxStackSize = 1;

  std::vector<EffectInfo> Effects;
};

struct ItemSpecialization {
  virtual std::shared_ptr<ItemEffect> createEffect() const = 0;
};

struct StatsBuffSpecialization : public ItemSpecialization {
  StatPoints StatsMin;
  StatPoints StatsMax;
  StatPoint MinPoints;
  StatPoint MaxPoints;

  std::shared_ptr<ItemEffect> createEffect() const override;
};

class ItemSpecializations {
public:
  struct SpecializationInfo {
    CapabilityFlags Flags;
    std::shared_ptr<ItemSpecialization> Specialization;
  };

public:
  void addSpecialization(CapabilityFlags Flags,
                         std::shared_ptr<ItemSpecialization> Spec);
  std::shared_ptr<ItemPrototype> actualize(const ItemPrototype &Proto) const;

private:
  std::vector<SpecializationInfo> Generators;
};

class Item {
public:
  Item(const ItemPrototype &Proto, int StackSize = 1,
       const std::shared_ptr<ItemPrototype> &Specialization = nullptr);
  virtual ~Item() = default;

  std::string getName() const;
  std::string getDescription() const;
  ItemType getType() const;
  int getMaxStackSize() const;
  std::vector<ItemPrototype::EffectInfo> getAllEffects() const;

  /// Returns true if other Item has same prototype and specialization
  bool isSameKind(const Item &Other) const;

  bool canApplyTo(const entt::entity &Entity, entt::registry &Reg,
                  CapabilityFlags Flags) const;
  void applyTo(const entt::entity &Entity, entt::registry &Reg,
               CapabilityFlags Flags) const;
  bool canRemoveFrom(const entt::entity &Entity, entt::registry &Reg,
                     CapabilityFlags Flags) const;
  void removeFrom(const entt::entity &Entity, entt::registry &Reg,
                  CapabilityFlags Flags) const;

private:
  const ItemPrototype &getProto() const;

public:
  int StackSize = 1;

private:
  const ItemPrototype *Proto = nullptr;
  std::shared_ptr<const ItemPrototype> Specialization = nullptr;
};

struct EquipmentSlot {
  const ItemType BaseTypeFilter = ItemType::None;
  std::optional<Item> It = std::nullopt;
  ItemType TypeFilter = ItemType::None;
};

class Equipment {
public:
  EquipmentSlot Ring = {ItemType::Ring};
  EquipmentSlot Amulet = {ItemType::Amulet};
  EquipmentSlot Helmet = {ItemType::Helmet};
  EquipmentSlot ChestPlate = {ItemType::ChestPlate};
  EquipmentSlot Pants = {ItemType::Pants};
  EquipmentSlot Boots = {ItemType::Boots};
  EquipmentSlot Weapon = {ItemType::Weapon};
  EquipmentSlot OffHand = {ItemType::OffHand};

  inline std::array<EquipmentSlot *, 8> all() {
    return {&Ring,  &Amulet, &Helmet, &ChestPlate,
            &Pants, &Boots,  &Weapon, &OffHand};
  }

  EquipmentSlot &getSlot(ItemType It);
  const EquipmentSlot &getSlot(ItemType It) const;

  bool isEquipped(ItemType Type) const;
  bool canEquip(ItemType Type) const;
  void equip(Item Item);
  Item unequip(ItemType Type);
};

} // namespace rogue

#endif // #ifndef ROGUE_ITEM_H