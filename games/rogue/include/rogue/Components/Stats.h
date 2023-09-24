#ifndef ROGUE_COMPONENTS_STATS_H
#define ROGUE_COMPONENTS_STATS_H

#include <algorithm>
#include <array>

namespace rogue {

using StatPoint = int;
using StatValue = double;

struct StatPoints {
  StatPoint Int = 0;
  StatPoint Str = 0;
  StatPoint Dex = 0;
  StatPoint Vit = 0;

  StatPoints &operator+=(const StatPoints &Other) {
    *this = {Int + Other.Int, Str + Other.Str, Dex + Other.Dex,
             Vit + Other.Vit};
    return *this;
  }

  StatPoints &operator-=(const StatPoints &Other) {
    *this = {Int - Other.Int, Str - Other.Str, Dex - Other.Dex,
             Vit - Other.Vit};
    return *this;
  }

  inline std::array<StatPoint *, 4> all() { return {{&Int, &Str, &Dex, &Vit}}; }

  inline std::array<const StatPoint *, 4> all() const {
    return {{&Int, &Str, &Dex, &Vit}};
  }
};

inline StatPoints operator-(const StatPoints &Lhs,
                            const StatPoints &Rhs) noexcept {
  auto Copy = Lhs;
  Copy -= Rhs;
  return Copy;
}

inline StatPoints operator+(const StatPoints &Lhs,
                            const StatPoints &Rhs) noexcept {
  auto Copy = Lhs;
  Copy += Rhs;
  return Copy;
}

struct StatsComp {
  StatPoints Base;
  StatPoints Bonus;

  inline void reset() { Bonus = {0, 0, 0, 0}; }
  inline void add(StatPoints Bonus) { this->Bonus += Bonus; }
  inline StatPoints effective() const { return Base + Bonus; }
};

struct ValueRegenCompBase {
  StatValue Value = 100;
  StatValue MaxValue = 100;

  unsigned TickPeriod = 4;
  unsigned TicksLeft = 1;
  StatValue RegenAmount = 0.1;

  StatValue restore(StatValue Amount);
  StatValue reduce(StatValue Amount);
};

struct HealthComp : public ValueRegenCompBase {};

struct ManaComp : public ValueRegenCompBase {};

struct AgilityComp {
  StatValue Agility = 30;
  StatValue AP = 0;
  StatValue MaxAP = 20;

  void gainAP(unsigned APAmount) { AP = std::min(MaxAP, AP + APAmount); }

  bool trySpendAP(unsigned APAmount) {
    if (AP < APAmount) {
      return false;
    }
    AP -= APAmount;
    return true;
  }
};

struct MeleeAttackComp {
  StatValue Damage = 10;
  StatValue APCost = 5;

  StatValue getEffectiveDamage(StatPoints SrcStats) const {
    auto Str = StatValue(SrcStats.Str);
    return (Damage + Str) * (100.0 + Str) / 100.0;
  }

  StatValue getEffectiveDamage(StatsComp *SrcSC) const {
    if (!SrcSC) {
      return Damage;
    }
    return getEffectiveDamage(SrcSC->effective());
  }
};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_STATS_H