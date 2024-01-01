#ifndef ROGUE_ITEM_SPECIALIZATION_H
#define ROGUE_ITEM_SPECIALIZATION_H

#include <memory>
#include <rogue/Components/Stats.h>
#include <rogue/EffectInfo.h>
#include <rogue/ItemType.h>
#include <vector>

namespace rogue {
class ItemEffect;
class ItemPrototype;
} // namespace rogue

namespace rogue {

class ItemSpecialization {
public:
  virtual ~ItemSpecialization() = default;
  virtual std::shared_ptr<ItemEffect> createEffect() const = 0;
};

class StatsBuffSpecialization : public ItemSpecialization {
public:
  StatPoint MinPoints = 0;
  StatPoint MaxPoints = 0;
  std::shared_ptr<ItemEffect> createEffect() const override;
};

class ItemSpecializations {
public:
  struct SpecializationInfo {
    EffectAttributes Attributes;
    std::shared_ptr<ItemSpecialization> Specialization;
  };

public:
  void addSpecialization(EffectAttributes Attributes,
                         std::shared_ptr<ItemSpecialization> Spec);
  std::shared_ptr<ItemPrototype> actualize(const ItemPrototype &Proto) const;

private:
  std::vector<SpecializationInfo> Generators;
};

} // namespace rogue

#endif // ROGUE_ITEM_SPECIALIZATION_H