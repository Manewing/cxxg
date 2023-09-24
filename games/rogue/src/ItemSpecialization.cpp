#include <rogue/Components/Buffs.h>
#include <rogue/ItemEffect.h>
#include <rogue/ItemPrototype.h>
#include <rogue/ItemSpecialization.h>

namespace rogue {

std::shared_ptr<ItemEffect> StatsBuffSpecialization::createEffect() const {
  // Compute points to spent
  assert(MinPoints <= MaxPoints);
  StatPoint Points = rand() % (MaxPoints - MinPoints + 1) + MinPoints;

  StatPoints Stats;
  auto AllStats = Stats.all();

  // Distribute points
  while (Points-- > 0) {
    auto *Stat = AllStats[rand() % AllStats.size()];
    *Stat += 1;
  }

  StatsBuffComp Buff;
  Buff.Bonus = Stats;
  return std::make_shared<ApplyBuffItemEffect<StatsBuffComp, StatsComp>>(Buff);
}

void ItemSpecializations::addSpecialization(
    CapabilityFlags Flags, std::shared_ptr<ItemSpecialization> Spec) {
  Generators.push_back({Flags, std::move(Spec)});
}

std::shared_ptr<ItemPrototype>
ItemSpecializations::actualize(const ItemPrototype &Proto) const {
  std::vector<EffectInfo> AllEffects;
  AllEffects.reserve(Generators.size());
  for (const auto &Gen : Generators) {
    AllEffects.push_back({Gen.Flags, Gen.Specialization->createEffect()});
  }
  return std::make_shared<ItemPrototype>(Proto.ItemId, Proto.Name,
                                         Proto.Description, Proto.Type,
                                         Proto.MaxStackSize, AllEffects);
}

} // namespace rogue
