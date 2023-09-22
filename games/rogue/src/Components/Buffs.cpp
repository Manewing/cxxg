#include <rogue/Components/Buffs.h>

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

void StatsBuffComp::add(const StatsBuffComp &Other) {
  AdditiveBuff::add();
  Bonus += Other.Bonus;
}

bool StatsBuffComp::remove(const StatsBuffComp &Other) {
  Bonus += Other.Bonus;
  return AdditiveBuff::remove(Other);
}

void StatsTimedBuffComp::add(const StatsTimedBuffComp &Other) {
  // Only one stats buff is allowed, can however boost all stats
  Bonus = Other.Bonus;
  TicksLeft = Other.TicksLeft;
}

std::string_view PoisonDebuffComp::getName() const { return "Poison debuff"; }

std::string_view BleedingDebuffComp::getName() const {
  return "Bleeding debuff";
}

std::string_view HealthRegenBuffComp::getName() const {
  return "Health regeneration buff";
}

std::string_view ManaRegenBuffComp::getName() const {
  return "Mana regeneration buff";
}

void copyBuffs(entt::entity EntityFrom, entt::registry &RegFrom,
               entt::entity EntityTo, entt::registry &RegTo) {
  copyComponents<BuffTypeList>(EntityFrom, RegFrom, EntityTo, RegTo);
}

void removeBuffs(entt::entity Entity, entt::registry &Reg) {
  removeComponents<BuffTypeList>(Entity, Reg);
}

} // namespace rogue