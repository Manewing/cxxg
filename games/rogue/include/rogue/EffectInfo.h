#ifndef ROGUE_EFFECT_INFO_H
#define ROGUE_EFFECT_INFO_H

#include <entt/entt.hpp>
#include <iosfwd>
#include <memory>
#include <rogue/ItemType.h>
#include <rogue/Types.h>

namespace rogue {
class ItemEffect;
}

namespace rogue {

/// Attributes to attach to an effect, determines how the effect is applied
struct EffectAttributes {
  /// Flags determining how the effect is applied
  CapabilityFlags Flags = CapabilityFlags::None;

  /// Action point cost to use the effect, if non-zero
  StatValue APCost = 0;

  /// Mana cost to use the effect, if non-zero
  StatValue ManaCost = 0;

  /// Health cost to use the effect, if non-zero
  StatValue HealthCost = 0;

  /// Adds from other attributes to this one
  void addFrom(const EffectAttributes &Other);

  /// Computes new costs from the combination of this and other attributes
  void updateCostsFrom(const EffectAttributes &Other);

  /// Checks if the entity has enough resources to use the effect
  bool checkCosts(const entt::entity &Entity, entt::registry &Reg) const;

  /// Applies the costs to the entity
  void applyCosts(const entt::entity &Entity, entt::registry &Reg) const;

  template <class Archive> void serialize(Archive &Ar) {
    Ar(Flags, APCost, ManaCost, HealthCost);
  }
};

inline bool operator==(const EffectAttributes &LHS,
                       const EffectAttributes &RHS) noexcept {
  return std::tie(LHS.Flags, LHS.APCost, LHS.ManaCost, LHS.HealthCost) ==
         std::tie(RHS.Flags, RHS.APCost, RHS.ManaCost, RHS.HealthCost);
}

std::ostream &operator<<(std::ostream &OS, const EffectAttributes &Attr);

struct EffectInfo {
  EffectAttributes Attributes;
  std::shared_ptr<ItemEffect> Effect;

  bool canApplyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg, CapabilityFlags Flags) const;
  void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
               entt::registry &Reg, CapabilityFlags Flags) const;
  bool canRemoveFrom(const entt::entity &SrcEt, const entt::entity &DstEt,
                     entt::registry &Reg, CapabilityFlags Flags) const;
  void removeFrom(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg, CapabilityFlags Flags) const;

  template <class Archive> void serialize(Archive &Ar) {
    Ar(Attributes, Effect);
  }
};

std::ostream &operator<<(std::ostream &OS, const EffectInfo &Info);

} // namespace rogue

#endif // #ifndef ROGUE_EFFECT_INFO_H