#ifndef ROGUE_COMPONENTS_BUFFS_H
#define ROGUE_COMPONENTS_BUFFS_H

#include <cereal/types/base_class.hpp>
#include <entt/entt.hpp>
#include <optional>
#include <rogue/Components/Helpers.h>
#include <rogue/Components/LOS.h>
#include <rogue/Components/Stats.h>
#include <string>
#include <string_view>

namespace rogue {

// All buffs that can be applied to an entity have to inherit from this class
struct BuffBase {
  static bool rollForPercentage(StatValue Percentage);

  virtual ~BuffBase() = default;

  /// Returns a name for the buff, e.g. "Poison Debuff"
  virtual std::string getName() const = 0;

  /// Returns a description of the buff, e.g. "Poisoned for 5 ticks"
  virtual std::string getDescription() const = 0;
};

struct AdditiveBuff {
  unsigned SourceCount = 1;
  void add(const AdditiveBuff &Other);
  bool remove(const AdditiveBuff &Other);

  template <class Archive> void serialize(Archive &Ar) { Ar(SourceCount); }
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

  template <class Archive> void serialize(Archive &Ar) {
    Ar(TicksLeft, TickPeriod, TickPeriodsLeft);
  }
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

  template <class Archive> void serialize(Archive &Ar) {
    Ar(cereal::base_class<TimedBuff>(this), TickAmount, RealDuration);
  }

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
  std::string getName() const override;
  std::string getDescription() const override;
  void add(const StatsBuffComp &Other);
  bool remove(const StatsBuffComp &Other);

  template <class Archive> void serialize(Archive &Ar) {
    Ar(cereal::base_class<AdditiveBuff>(this), Bonus);
  }

public:
  StatPoints Bonus;
};

struct StatsTimedBuffComp : public StatsBuffComp, public TimedBuff {
  std::string getName() const override;
  std::string getDescription() const override;

  /// Will increase the buff but the duration will be minimum of the two
  void add(const StatsTimedBuffComp &Other);

  /// Removes an added buff, returns true if component can be removed
  bool remove(const StatsTimedBuffComp &Other);

  template <class Archive> void serialize(Archive &Ar) {
    Ar(cereal::base_class<StatsBuffComp>(this),
       cereal::base_class<TimedBuff>(this));
  }
};

// TODO:
//  Buffs:
//      - Stat buff, Stat.X + Y
//  Debuffs:
//      - Burning
//      - Slow

struct PoisonDebuffComp : public DiminishingReturnsValueGenBuff,
                          public BuffBase {
public:
  std::string getName() const override;
  std::string getApplyDesc() const override;
  std::string getDescription() const override;
};

struct BleedingDebuffComp : public DiminishingReturnsValueGenBuff,
                            public BuffBase {
public:
  std::string getName() const override;
  std::string getApplyDesc() const override;
  std::string getDescription() const override;
};

struct HealthRegenBuffComp : public DiminishingReturnsValueGenBuff,
                             public BuffBase {
public:
  std::string getName() const override;
  std::string getApplyDesc() const override;
  std::string getDescription() const override;
};

struct ManaRegenBuffComp : public DiminishingReturnsValueGenBuff,
                           public BuffBase {
public:
  std::string getName() const override;
  std::string getApplyDesc() const override;
  std::string getDescription() const override;
};

struct BlindedDebuffComp : public TimedBuff, public BuffBase {
public:
  /// Factor by which the line of sight range is reduced
  static constexpr const StatValue Factor = 0.1;

public:
  std::string getName() const override;
  std::string getDescription() const override;
  void add(const BlindedDebuffComp &Other);
};

struct MindVisionBuffComp : public TimedBuff, public BuffBase {
public:
  std::string getName() const override;
  std::string getDescription() const override;
  void add(const MindVisionBuffComp &Other);

  template <class Archive> void serialize(Archive &Ar) {
    Ar(cereal::base_class<TimedBuff>(this), Range);
  }

public:
  unsigned Range = 100;
};

struct InvisibilityBuffComp : public TimedBuff, public BuffBase {
public:
  std::string getName() const override;
  std::string getDescription() const override;
  void add(const InvisibilityBuffComp &Other);
};

struct ArmorBuffComp : public AdditiveBuff, public BuffBase {
  StatValue PhysArmor = 0;
  StatValue MagicArmor = 0;

  std::string getName() const override;
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

  template <class Archive> void serialize(Archive &Ar) {
    Ar(cereal::base_class<AdditiveBuff>(this), PhysArmor, MagicArmor);
  }
};

struct BlockBuffComp : public AdditiveBuff, public BuffBase {
  /// Block chance in percent
  StatValue BlockChance = 5.0;

  std::string getName() const override;
  std::string getDescription() const override;

  void add(const BlockBuffComp &Other);
  bool remove(const BlockBuffComp &Other);

  template <class Archive> void serialize(Archive &Ar) {
    Ar(cereal::base_class<AdditiveBuff>(this), BlockChance);
  }
};

class BuffApplyHelperBase {
public:
  /// Marks the entities as in combat
  static void markCombat(const entt::entity &SrcEt, const entt::entity &DstEt,
                         entt::registry &Reg);

  static void publishBuffAppliedEvent(const entt::entity &SrcEt,
                                      const entt::entity &DstEt, bool IsCombat,
                                      entt::registry &Reg,
                                      const BuffBase &Buff);

public:
  virtual ~BuffApplyHelperBase() = default;

protected:
  BuffApplyHelperBase() = default;
};

template <typename BuffType, bool IsCombat, typename... RequiredComps>
class BuffApplyHelper : public BuffApplyHelperBase {
public:
  static bool canApplyTo(const entt::entity &, const entt::entity &DstEt,
                         entt::registry &Reg) {
    if constexpr (sizeof...(RequiredComps) == 0) {
      return true;
    } else {
      return Reg.all_of<RequiredComps...>(DstEt);
    }
  }

  static void applyTo(const BuffType &Buff, const entt::entity &SrcEt,
                      const entt::entity &DstEt, entt::registry &Reg) {
    auto ExistingBuff = Reg.try_get<BuffType>(DstEt);
    if (ExistingBuff) {
      ExistingBuff->add(Buff);
    } else {
      Reg.emplace<BuffType>(DstEt, Buff);
    }
    if constexpr (IsCombat) {
      markCombat(SrcEt, DstEt, Reg);
    }
    publishBuffAppliedEvent(SrcEt, DstEt, IsCombat, Reg, Buff);
  }

  static bool canRemoveFrom(const entt::entity &, const entt::entity &DstEt,
                            entt::registry &Reg) {
    if constexpr (sizeof...(RequiredComps) == 0) {
      return true;
    } else {
      return Reg.all_of<RequiredComps...>(DstEt);
    }
  }

  static void removeFrom(const BuffType &Buff, const entt::entity &SrcEt,
                         const entt::entity &DstEt, entt::registry &Reg) {
    auto ExistingBuff = Reg.try_get<BuffType>(DstEt);
    if (ExistingBuff) {
      if (ExistingBuff->remove(Buff)) {
        Reg.erase<BuffType>(DstEt);
      }
    }
    if constexpr (IsCombat) {
      markCombat(SrcEt, DstEt, Reg);
    }
  }
};

/// Chance on hit to apply a buff
template <typename BuffType, typename... RequiredComps>
struct ChanceToApplyBuffComp : public AdditiveBuff, public BuffBase {
  using Helper = BuffApplyHelper<BuffType, true, RequiredComps...>;

  /// Chance to apply buff in percent
  StatValue Chance = 5.0;

  /// Buff to apply
  BuffType Buff;

  std::string getName() const override {
    if (Chance != 100) {
      return std::to_string(int(Chance)) + "% chance on hit " + Buff.getName();
    }
    return std::to_string(int(Chance)) + "On hit " + Buff.getName();
  }

  std::string getDescription() const override {
    if (Chance != 100) {
      return std::to_string(int(Chance)) +
             "% chance on hit to apply: " + Buff.getDescription();
    }
    return "On hit apply " + Buff.getDescription();
  }

  void add(const ChanceToApplyBuffComp &Other) {
    AdditiveBuff::add(Other);
    Buff.add(Other.Buff);
  }

  bool remove(const ChanceToApplyBuffComp &Other) {
    if (!AdditiveBuff::remove(Other)) {
      Buff.remove(Other.Buff);
      return false;
    }
    return true;
  }

  bool canApplyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                  entt::registry &Reg) const {
    return Helper::canApplyTo(SrcEt, DstEt, Reg);
  }

  void applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
               entt::registry &Reg) const {
    if (!rollForPercentage(Chance)) {
      return;
    }
    Helper::applyTo(Buff, SrcEt, DstEt, Reg);
  }

  template <class Archive> void serialize(Archive &Ar) {
    Ar(cereal::base_class<AdditiveBuff>(this), Chance, Buff);
  }
};

/// Applies a time based stats buff every time the entity hits sth, only one can
/// be active
struct StatsBuffPerHitComp : public TimedBuff, public BuffBase {
  std::string getName() const override;
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

  template <class Archive> void serialize(Archive &Ar) {
    Ar(cereal::base_class<TimedBuff>(this), Stacks, MaxStacks, SBC);
  }
};

struct LifeStealBuffComp : public AdditiveBuff, public BuffBase {
  std::string getName() const override;
  std::string getDescription() const override;

  void add(const LifeStealBuffComp &Other);
  bool remove(const LifeStealBuffComp &Other);

  StatValue getEffectiveLifeSteal(StatValue Damage) const;

  StatValue Percent = 0.0;
  StatValue BonusHP = 0.0;

  template <class Archive> void serialize(Archive &Ar) {
    Ar(cereal::base_class<AdditiveBuff>(this), Percent, BonusHP);
  }
};

// Chance to apply on hit to target
using CoHTargetPoisonDebuffComp =
    ChanceToApplyBuffComp<PoisonDebuffComp, HealthComp>;
using CoHTargetBleedingDebuffComp =
    ChanceToApplyBuffComp<BleedingDebuffComp, HealthComp>;
using CoHTargetBlindedDebuffComp =
    ChanceToApplyBuffComp<BlindedDebuffComp, LineOfSightComp>;

// clang-format off
// Keep this list sorted alphabetically
using BuffTypeList = ComponentList<
 ArmorBuffComp,
 BleedingDebuffComp,
 BlindedDebuffComp,
 BlockBuffComp,
 CoHTargetBleedingDebuffComp,
 CoHTargetBlindedDebuffComp,
 CoHTargetPoisonDebuffComp,
 HealthRegenBuffComp,
 InvisibilityBuffComp,
 LifeStealBuffComp,
 ManaRegenBuffComp,
 MindVisionBuffComp,
 PoisonDebuffComp,
 StatsBuffComp,
 StatsBuffPerHitComp,
 StatsTimedBuffComp
>;
// clang-format on

void copyBuffs(entt::entity EntityFrom, entt::registry &RegFrom,
               entt::entity EntityTo, entt::registry &RegTo);

void removeBuffs(entt::entity Entity, entt::registry &Reg);

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_BUFFS_H