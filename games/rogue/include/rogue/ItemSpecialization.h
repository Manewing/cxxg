#ifndef ROGUE_ITEM_SPECIALIZATION_H
#define ROGUE_ITEM_SPECIALIZATION_H

#include <rogue/Components/Stats.h>
#include <rogue/ItemType.h>
#include <vector>

namespace rogue {
class ItemPrototype;
}

namespace rogue {

struct ItemSpecialization {
  virtual ~ItemSpecialization() = default;
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