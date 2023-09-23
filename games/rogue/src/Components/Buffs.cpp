#include <rogue/Components/Buffs.h>
#include <sstream>

namespace rogue {

void AdditiveBuff::add() { SourceCount++; }

bool AdditiveBuff::remove(const AdditiveBuff &) { return --SourceCount == 0; }

void RegenerationBuff::add(const RegenerationBuff &Other) {
  if (Other.total() > total()) {
    RegenAmount = Other.RegenAmount;
    TicksLeft = Other.TicksLeft;
  }
}

StatValue RegenerationBuff::total() const { return RegenAmount * TicksLeft; }

void ReductionBuff::add(const ReductionBuff &Other) {
  if (Other.total() > total()) {
    ReduceAmount = Other.ReduceAmount;
    TicksLeft = Other.TicksLeft;
  }
}

StatValue ReductionBuff::total() const { return ReduceAmount * TicksLeft; }

std::string_view StatsBuffComp::getName() const { return "Stats buff"; }

std::string StatsBuffComp::getDescription() const {
  std::stringstream SS;
  const char *Pred = "";
  if (Bonus.Str > 0) {
    SS << Pred << "Str +" << Bonus.Str;
    Pred = ", ";
  }
  if (Bonus.Dex > 0) {
    SS << Pred << "Dex +" << Bonus.Dex;
    Pred = ", ";
  }
  if (Bonus.Int > 0) {
    SS << Pred << "Int +" << Bonus.Int;
    Pred = ", ";
  }
  if (Bonus.Vit > 0) {
    SS << Pred << "Vit +" << Bonus.Vit;
    Pred = ", ";
  }
  auto DescStr = SS.str();
  if (DescStr.empty()) {
    return "Nothing";
  }
  return DescStr;
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
  AdditiveBuff::add();
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

std::string PoisonDebuffComp::getDescription() const {
  std::stringstream SS;
  SS << "Poison reduces health by " << ReduceAmount << " for "
     << TicksLeft << " ticks";
  return SS.str();
}

std::string_view BleedingDebuffComp::getName() const {
  return "Bleeding debuff";
}

std::string BleedingDebuffComp::getDescription() const {
  std::stringstream SS;
  SS << "Bleeding reduces health by " << ReduceAmount << " for "
     << TicksLeft << " ticks";
  return SS.str();
}

std::string_view HealthRegenBuffComp::getName() const {
  return "Health regeneration buff";
}

std::string HealthRegenBuffComp::getDescription() const {
  std::stringstream SS;
  SS << "Health regeneration increased by " << RegenAmount << " for "
     << TicksLeft << " ticks";
  return SS.str();
}

std::string_view ManaRegenBuffComp::getName() const {
  return "Mana regeneration buff";
}

std::string ManaRegenBuffComp::getDescription() const {
  std::stringstream SS;
  SS << "Mana regeneration increased by " << RegenAmount << " for "
     << TicksLeft << " ticks";
  return SS.str();
}

void copyBuffs(entt::entity EntityFrom, entt::registry &RegFrom,
               entt::entity EntityTo, entt::registry &RegTo) {
  copyComponents<BuffTypeList>(EntityFrom, RegFrom, EntityTo, RegTo);
}

void removeBuffs(entt::entity Entity, entt::registry &Reg) {
  removeComponents<BuffTypeList>(Entity, Reg);
}

} // namespace rogue