#ifndef ROGUE_COMPONENTS_BUFFS_H
#define ROGUE_COMPONENTS_BUFFS_H

#include <rogue/Components/Stats.h>
#include <string_view>

namespace rogue {

struct AdditiveBuff {
  unsigned SourceCount = 1;
  void add();
  bool remove(const AdditiveBuff &Other);
};

struct TimedBuff {
  unsigned TicksLeft = 10;
  bool remove(const TimedBuff&) { return false; }
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

struct StatsBuffComp : public AdditiveBuff {
public:
  std::string_view getName() const;
  void add(const StatsBuffComp &Other);
  bool remove(const StatsBuffComp &Other);

public:
  StatPoints Bonus;
};

struct StatsTimedBuffComp : public StatsBuffComp, public TimedBuff {
  std::string_view getName() const;
  void add(const StatsTimedBuffComp &Other);
};

// TODO:
//  Buffs:
//      - Stat buff, Stat.X + Y
//  Debuffs:
//      - Burning
//      - Slow
//      - Blinded

struct PoisonDebuffComp : public ReductionBuff {
public:
  std::string_view getName() const;
};

struct BleedingDebuffComp : public ReductionBuff {
public:
  std::string_view getName() const;
};

struct HealthRegenBuffComp : public RegenerationBuff {
public:
  std::string_view getName() const;
};

struct ManaRegenBuffComp : public RegenerationBuff {
public:
  std::string_view getName() const;
};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_BUFFS_H