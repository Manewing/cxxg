#ifndef ROGUE_COMPONENTS_BUFFS_H
#define ROGUE_COMPONENTS_BUFFS_H

#include <entt/entt.hpp>
#include <optional>
#include <rogue/Components/Helpers.h>
#include <rogue/Components/Stats.h>
#include <string>
#include <string_view>

namespace rogue {

// All buffs that can be applied to an entity have to inherit from this class
struct BuffBase {
  virtual ~BuffBase() = default;

  /// Returns a name for the buff, e.g. "Poison Debuff"
  virtual std::string_view getName() const = 0;

  /// Returns a description of the buff, e.g. "Poisoned for 5 ticks"
  virtual std::string getDescription() const = 0;
};

struct AdditiveBuff {
  unsigned SourceCount = 1;
  void add(const AdditiveBuff &Other);
  bool remove(const AdditiveBuff &Other);
};

struct TimedBuff {
  enum class State { Expired, Waiting, Active };

  unsigned TicksLeft = 0;
  unsigned TickPeriod = 0;
  unsigned TickPeriodsLeft = 0;

  /// Post-decrement to match count, returns true when hitting zero
  virtual State tick();

  unsigned totalTicksLeft() const;

  bool remove(const TimedBuff &) { return false; }
};

struct DiminishingReturnsValueGenBuff : public TimedBuff {
public:
  DiminishingReturnsValueGenBuff();
  DiminishingReturnsValueGenBuff(StatValue TickAmount, StatValue Duration,
                                 unsigned TickPeriod);
  void init(StatValue TickAmount, StatValue Duration, unsigned TickPeriod);
  void add(const DiminishingReturnsValueGenBuff &Other);

  State tick() override;

  /// Returns total amount of regeneration left
  StatValue total() const;

  virtual std::string getApplyDesc() const = 0;

protected:
  std::string getParamApplyDesc(std::string_view Prologue,
                                std::string_view Epilogue,
                                std::string_view ValuePointName) const;
  std::string getParamDescription(std::string_view Prologue,
                                  std::string_view Epilogue,
                                  std::string_view ValuePointName) const;

public:
  /// Amount of regeneration/reduction per tick period
  StatValue TickAmount = 0.0;

  /// Real total amount of regeneration/reduction kept to avoid integer rounding
  /// errors
  StatValue RealDuration = 0.0;
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

struct PoisonDebuffComp : public DiminishingReturnsValueGenBuff,
                          public BuffBase {
public:
  std::string_view getName() const override;
  std::string getApplyDesc() const override;
  std::string getDescription() const override;
};

struct BleedingDebuffComp : public DiminishingReturnsValueGenBuff,
                            public BuffBase {
public:
  std::string_view getName() const override;
  std::string getApplyDesc() const override;
  std::string getDescription() const override;
};

struct HealthRegenBuffComp : public DiminishingReturnsValueGenBuff,
                             public BuffBase {
public:
  std::string_view getName() const override;
  std::string getApplyDesc() const override;
  std::string getDescription() const override;
};

struct ManaRegenBuffComp : public DiminishingReturnsValueGenBuff,
                           public BuffBase {
public:
  std::string_view getName() const override;
  std::string getApplyDesc() const override;
  std::string getDescription() const override;
};

struct BlindedDebuffComp : public TimedBuff, public BuffBase {
public:
  /// Factor by which the line of sight range is reduced
  static constexpr const StatValue Factor = 0.1;

public:
  std::string_view getName() const override;
  std::string getDescription() const override;
  void add(const BlindedDebuffComp &Other);
};

struct ArmorBuffComp : public AdditiveBuff, public BuffBase {
  StatValue PhysArmor = 0;
  StatValue MagicArmor = 0;

  std::string_view getName() const override;
  std::string getDescription() const override;

  void add(const ArmorBuffComp &Other);
  bool remove(const ArmorBuffComp &Other);

  // Vitality increases armor by 0.5% per point
  StatValue getPhysEffectiveArmor(StatPoints DstStats) const;

  // Intelligence increases magic armor by 0.5% per point
  StatValue getMagicEffectiveArmor(StatPoints DstStats) const;

  // Armor reduces damage by 1/x where x is armor * 0.5
  StatValue getPhysEffectiveDamage(StatValue Damage, StatsComp *DstSC) const;

  // Magic armor reduces damage by 1/x where x is armor * 0.5
  StatValue getMagicEffectiveDamage(StatValue Damage, StatsComp *DstSC) const;
};

struct BlockBuffComp : public AdditiveBuff, public BuffBase {
  /// Block chance in percent
  StatValue BlockChance = 5.0;

  std::string_view getName() const override;
  std::string getDescription() const override;

  void add(const BlockBuffComp &Other);
  bool remove(const BlockBuffComp &Other);
};

/// Applies a time based stats buff every time the entity hits sth, only one can
/// be active
struct StatsBuffPerHitComp : public TimedBuff, public BuffBase {
  std::string_view getName() const override;
  std::string getDescription() const override;

  void add(const StatsBuffPerHitComp &Other);
  bool remove(const StatsBuffPerHitComp &Other);

  State tick() override;

  bool addStack();

  StatsBuffComp getEffectiveBuff(StatPoint Stack) const;

  StatPoint Stacks = 0;
  StatPoint MaxStacks = 0;
  StatsBuffComp SBC;

  StatsBuffComp *Applied = nullptr;
  std::optional<StatPoint> AppliedStack = std::nullopt;
};

using BuffTypeList =
    ComponentList<StatsBuffComp, StatsTimedBuffComp, PoisonDebuffComp,
                  BleedingDebuffComp, HealthRegenBuffComp, ManaRegenBuffComp,
                  BlindedDebuffComp, ArmorBuffComp, BlockBuffComp,
                  StatsBuffPerHitComp>;

void copyBuffs(entt::entity EntityFrom, entt::registry &RegFrom,
               entt::entity EntityTo, entt::registry &RegTo);

void removeBuffs(entt::entity Entity, entt::registry &Reg);

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_BUFFS_H