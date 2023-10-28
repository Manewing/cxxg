#include <iomanip>
#include <rogue/Components/Buffs.h>
#include <sstream>
#include <string_view>

namespace rogue {

void AdditiveBuff::add(const AdditiveBuff &Other) {
  SourceCount += Other.SourceCount;
}

bool AdditiveBuff::remove(const AdditiveBuff &Other) {
  assert(SourceCount >= Other.SourceCount);
  SourceCount -= Other.SourceCount;
  return SourceCount == 0;
}

TimedBuff::State TimedBuff::tick() {
  if (TicksLeft-- != 0) {
    return State::Waiting;
  }
  if (TickPeriodsLeft-- != 0) {
    TicksLeft = TickPeriod;
    return State::Active;
  }
  TicksLeft = 0;
  TickPeriodsLeft = 0;
  return State::Expired;
}

unsigned TimedBuff::totalTicksLeft() const {
  return TicksLeft + TickPeriodsLeft * TickPeriod;
}

namespace {

/// Computes diminishing returns when adding a value to a start value
float computeDiminishingReturns(float StartValue, float AdditionalValue,
                                float Coefficient) {
  return std::max(StartValue, AdditionalValue) +
         std::min(StartValue, AdditionalValue) /
             (std::max(-std::min(.0f, StartValue - AdditionalValue),
                       StartValue / AdditionalValue) +
              Coefficient);
}

} // namespace

void DiminishingReturnsValueGenBuff::add(
    const DiminishingReturnsValueGenBuff &Other) {
  // Make apply immediate on next tick
  TicksLeft = 0;

  // Compute diminishing returns for reduction amount per tick period
  TickAmount = computeDiminishingReturns(TickAmount, Other.TickAmount,
                                         /*Coefficient=*/40.0f);

  // Compute diminishing returns for total amount of reduction
  RealDuration = computeDiminishingReturns(RealDuration, Other.RealDuration,
                                           /*Coefficient=*/20.0f);

  // Compute new tick periods left, tick period is kept from existing buff
  TickPeriodsLeft = RealDuration / TickPeriod;
}

StatValue DiminishingReturnsValueGenBuff::total() const {
  return TickAmount * TickPeriodsLeft;
}

std::string DiminishingReturnsValueGenBuff::getApplyDesc() const {
  return getParamApplyDesc("Reduce ", "", "HP");
}

std::string DiminishingReturnsValueGenBuff::getParamApplyDesc(
    std::string_view Prologue, std::string_view Epilogue,
    std::string_view ValuePointName) const {
  std::stringstream SS;
  SS << Prologue << std::setprecision(2) << TickAmount << " " << ValuePointName
     << Epilogue;
  return SS.str();
}

std::string DiminishingReturnsValueGenBuff::getParamDescription(
    std::string_view Prologue, std::string_view Epilogue,
    std::string_view ValuePointName) const {
  std::stringstream SS;
  SS << Prologue << std::setprecision(2) << TickAmount << " " << ValuePointName
     << " every " << TickPeriod << " ticks, for " << totalTicksLeft()
     << " ticks (" << total() << ValuePointName << ")" << Epilogue;
  return SS.str();
}

std::string_view StatsBuffComp::getName() const { return "Stats buff"; }

std::string StatsBuffComp::getDescription() const {
  std::stringstream SS;
  Bonus.dump(SS, /*DumpZero=*/false);
  return SS.str();
}

std::string_view StatsTimedBuffComp::getName() const {
  return "Timed stats buff";
}

std::string StatsTimedBuffComp::getDescription() const {
  std::stringstream SS;
  SS << StatsBuffComp::getDescription() << " for " << TicksLeft << " ticks";
  return SS.str();
}

void StatsBuffComp::add(const StatsBuffComp &Other) {
  AdditiveBuff::add(Other);
  Bonus += Other.Bonus;
}

bool StatsBuffComp::remove(const StatsBuffComp &Other) {
  Bonus -= Other.Bonus;
  return AdditiveBuff::remove(Other);
}

void StatsTimedBuffComp::add(const StatsTimedBuffComp &Other) {
  // Only one stats buff is allowed, can however boost all stats
  Bonus = Other.Bonus;
  TicksLeft = Other.TicksLeft;
}

std::string_view PoisonDebuffComp::getName() const { return "Poison debuff"; }

std::string PoisonDebuffComp::getApplyDesc() const {
  return getParamApplyDesc("Poison damage taken ", "", "HP");
}

std::string PoisonDebuffComp::getDescription() const {
  return getParamDescription("Poison reduces health by ", "", "HP");
}

std::string_view BleedingDebuffComp::getName() const {
  return "Bleeding debuff";
}

std::string BleedingDebuffComp::getApplyDesc() const {
  return getParamApplyDesc("Bleed health by ", "", "HP");
}

std::string BleedingDebuffComp::getDescription() const {
  return getParamDescription("Bleeding reduces health by ", "", "HP");
}

std::string_view HealthRegenBuffComp::getName() const {
  return "Health regeneration buff";
}

std::string HealthRegenBuffComp::getApplyDesc() const {
  return getParamApplyDesc("Health increased by ", "", "HP");
}

std::string HealthRegenBuffComp::getDescription() const {
  return getParamDescription("Health increased by ", "", "HP");
}

std::string_view ManaRegenBuffComp::getName() const {
  return "Mana regeneration buff";
}

std::string ManaRegenBuffComp::getApplyDesc() const {
  return getParamApplyDesc("Mana increased by ", "", "MP");
}

std::string ManaRegenBuffComp::getDescription() const {
  return getParamDescription("Mana increased by ", "", "MP");
}

std::string_view ArmorBuffComp::getName() const { return "Armor buff"; }

std::string ArmorBuffComp::getDescription() const {
  std::stringstream SS;
  const char *Pred = "";
  if (BaseArmor > 0) {
    SS << Pred << "+ " << BaseArmor << " Armor";
    Pred = ", ";
  }
  if (MagicArmor > 0) {
    SS << Pred << "+ " << MagicArmor << " Magic Armor";
    Pred = ", ";
  }
  auto Str = SS.str();
  if (Str.empty()) {
    return "Nothing";
  }
  return Str;
}

void ArmorBuffComp::add(const ArmorBuffComp &Other) {
  AdditiveBuff::add(Other);
  BaseArmor += Other.BaseArmor;
  MagicArmor += Other.MagicArmor;
}

bool ArmorBuffComp::remove(const ArmorBuffComp &Other) {
  BaseArmor -= Other.BaseArmor;
  MagicArmor -= Other.MagicArmor;
  return AdditiveBuff::remove(Other);
}

StatValue ArmorBuffComp::getEffectiveArmor(StatPoints DstStats) const {
  auto Vit = StatValue(DstStats.Vit);
  return BaseArmor * (1000.0 + Vit * 5) / 1000.0;
}

StatValue ArmorBuffComp::getEffectiveDamage(StatValue Damage,
                                            StatsComp *DstSC) const {
  auto Armor = BaseArmor;
  if (DstSC) {
    Armor = getEffectiveArmor(DstSC->effective());
  }
  return Damage * 100.0 / (100.0 + Armor * 0.5);
}

std::string_view StatsBuffPerHitComp::getName() const {
  return "Stats buff per hit";
}

std::string StatsBuffPerHitComp::getDescription() const {
  std::stringstream SS;
  SS << "Upon hit: " << SBC.getDescription() << " for " << MaxTicks
     << " ticks,  max stacks " << MaxStacks;
  if (Stacks) {
    SS << " (" << Stacks << ");\n    (";
    getEffectiveBuff(Stacks).Bonus.dump(SS, false);
    SS << ") " << TicksLeft << " ticks left";
  }

  return SS.str();
}

void StatsBuffPerHitComp::add(const StatsBuffPerHitComp &Other) {
  *this = Other;
}

bool StatsBuffPerHitComp::remove(const StatsBuffPerHitComp &) {
  if (AppliedStack) {
    Applied->remove(getEffectiveBuff(*AppliedStack));
    AppliedStack = std::nullopt;
    Applied = nullptr;
  }
  return true;
}

bool StatsBuffPerHitComp::tick() {
  auto St = TimedBuff::tick();
  bool Expired = St == TimedBuff::State::Expired;
  if (Expired) {
    Stacks = 0;
  }
  return Expired;
}

bool StatsBuffPerHitComp::addStack() {
  TicksLeft = MaxTicks;
  if (Stacks < MaxStacks) {
    Stacks++;
    return true;
  }
  return false;
}

StatsBuffComp StatsBuffPerHitComp::getEffectiveBuff(StatPoint S) const {
  StatsBuffComp B;
  for (int I = 0; I < S; ++I) {
    B.add(SBC);
  }
  B.SourceCount = 1;
  return B;
}

void copyBuffs(entt::entity EntityFrom, entt::registry &RegFrom,
               entt::entity EntityTo, entt::registry &RegTo) {
  copyComponents<BuffTypeList>(EntityFrom, RegFrom, EntityTo, RegTo);
}

void removeBuffs(entt::entity Entity, entt::registry &Reg) {
  removeComponents<BuffTypeList>(Entity, Reg);
}

} // namespace rogue