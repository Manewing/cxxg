#include <rogue/Components/Buffs.h>
#include <sstream>
#include <string_view>

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
  SS << "Poison reduces health by " << ReduceAmount << " for " << TicksLeft
     << " ticks";
  return SS.str();
}

std::string_view BleedingDebuffComp::getName() const {
  return "Bleeding debuff";
}

std::string BleedingDebuffComp::getDescription() const {
  std::stringstream SS;
  SS << "Bleeding reduces health by " << ReduceAmount << " for " << TicksLeft
     << " ticks";
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
  SS << "Mana regeneration increased by " << RegenAmount << " for " << TicksLeft
     << " ticks";
  return SS.str();
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
  AdditiveBuff::add();
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

void copyBuffs(entt::entity EntityFrom, entt::registry &RegFrom,
               entt::entity EntityTo, entt::registry &RegTo) {
  copyComponents<BuffTypeList>(EntityFrom, RegFrom, EntityTo, RegTo);
}

void removeBuffs(entt::entity Entity, entt::registry &Reg) {
  removeComponents<BuffTypeList>(Entity, Reg);
}

} // namespace rogue