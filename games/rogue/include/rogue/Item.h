#ifndef ROGUE_ITEM_H
#define ROGUE_ITEM_H

#include <array>
#include <entt/entt.hpp>
#include <memory>
#include <rogue/EffectInfo.h>
#include <rogue/ItemType.h>
#include <string>
#include <vector>

namespace rogue {
class ItemPrototype;
}

namespace rogue {

class Item {
public:
  explicit Item(const ItemPrototype &Proto, int StackSize = 1,
                const std::shared_ptr<ItemPrototype> &Specialization = nullptr,
                bool SpecOverrides = false);
  virtual ~Item() = default;

  int getId() const;
  const std::string &getName() const;
  std::string getQualifierName() const;
  std::string getDescription() const;
  ItemType getType() const;
  int getMaxStackSize() const;

  std::vector<EffectInfo> getAllEffects() const;

  bool hasEffect(CapabilityFlags Flags, bool AllowNull = false,
                 bool AllowRemove = false) const;

  CapabilityFlags getCapabilityFlags() const;

  /// Returns true if other Item has same prototype and specialization
  bool isSameKind(const Item &Other) const;

  bool canApplyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg, CapabilityFlags Flags) const;
  void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
               entt::registry &Reg, CapabilityFlags Flags) const;
  bool canRemoveFrom(const entt::entity &SrcEt, const entt::entity &DstEt,
                     entt::registry &Reg, CapabilityFlags Flags) const;
  void removeFrom(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg, CapabilityFlags Flags) const;

  const ItemPrototype &getProto() const;

public:
  int StackSize = 1;

private:
  const ItemPrototype *Proto = nullptr;
  std::shared_ptr<const ItemPrototype> Specialization = nullptr;
  bool SpecOverrides = false;
};

} // namespace rogue

#endif // #ifndef ROGUE_ITEM_H