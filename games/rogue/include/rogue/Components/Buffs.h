#ifndef ROGUE_COMPONENTS_BUFFS_H
#define ROGUE_COMPONENTS_BUFFS_H

#include <entt/entt.hpp>
#include <rogue/Components/Helpers.h>
#include <rogue/Components/Stats.h>
#include <string_view>

namespace rogue {

// All buffs that can be applied to an entity have to inherit from this class
struct BuffBase {
  virtual ~BuffBase() = default;
  virtual std::string_view getName() const = 0;
  virtual std::string getDescription() const = 0;
};

struct AdditiveBuff {
  unsigned SourceCount = 1;
  void add();
  bool remove(const AdditiveBuff &Other);
};

struct TimedBuff {
  unsigned TicksLeft = 10;
  bool remove(const TimedBuff &) { return false; }
};

struct RegenerationBuff : public TimedBuff {
public:
  void add(const RegenerationBuff &Other);
  StatValue total() const;

public:
  StatValue RegenAmount = 0.5;
};

struct ReductionBuff : public TimedBuff {
public:
  void add(const ReductionBuff &Other);
  StatValue total() const;

public:
  StatValue ReduceAmount = 0.5;
};

struct StatsBuffComp : public AdditiveBuff, public BuffBase {
public:
  std::string_view getName() const override;
  std::string getDescription() const override;
  void add(const StatsBuffComp &Other);
  bool remove(const StatsBuffComp &Other);

public:
  StatPoints Bonus;
};

struct StatsTimedBuffComp : public StatsBuffComp, public TimedBuff {
  std::string_view getName() const override;
  std::string getDescription() const override;
  void add(const StatsTimedBuffComp &Other);
};

// TODO:
//  Buffs:
//      - Stat buff, Stat.X + Y
//  Debuffs:
//      - Burning
//      - Slow
//      - Blinded

struct PoisonDebuffComp : public ReductionBuff, public BuffBase {
public:
  std::string_view getName() const override;
  std::string getDescription() const override;
};

struct BleedingDebuffComp : public ReductionBuff, public BuffBase {
public:
  std::string_view getName() const override;
  std::string getDescription() const override;
};

struct HealthRegenBuffComp : public RegenerationBuff, public BuffBase {
public:
  std::string_view getName() const override;
  std::string getDescription() const override;
};

struct ManaRegenBuffComp : public RegenerationBuff, public BuffBase {
public:
  std::string_view getName() const override;
  std::string getDescription() const override;
};

struct ArmorBuffComp : public AdditiveBuff, public BuffBase {
  StatValue BaseArmor = 0;
  StatValue MagicArmor = 0;  // FIXME not yet used

  std::string_view getName() const override;
  std::string getDescription() const override;

  void add(const ArmorBuffComp &Other);
  bool remove(const ArmorBuffComp &Other);

  // Vitality increases armor by 0.5% per point
  StatValue getEffectiveArmor(StatPoints DstStats) const;

  // Armor reduces damage by 1/x where x is armor * 0.5
  StatValue getEffectiveDamage(StatValue Damage, StatsComp *DstSC) const;
};

using BuffTypeList =
    ComponentList<StatsBuffComp, StatsTimedBuffComp, PoisonDebuffComp,
                  BleedingDebuffComp, HealthRegenBuffComp, ManaRegenBuffComp,
                  ArmorBuffComp>;

void copyBuffs(entt::entity EntityFrom, entt::registry &RegFrom,
               entt::entity EntityTo, entt::registry &RegTo);

void removeBuffs(entt::entity Entity, entt::registry &Reg);

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_BUFFS_H