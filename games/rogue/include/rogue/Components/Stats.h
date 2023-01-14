#ifndef ROGUE_COMPONENTS_STATS_H
#define ROGUE_COMPONENTS_STATS_H

#include <algorithm>

namespace rogue {

using StatPoint = int;
using StatValue = double;

struct StatValues {
  StatPoint Int = 11;
  StatPoint Str = 11;
  StatPoint Dex = 11;
  StatPoint Vit = 11;

  StatValues &operator+=(const StatValues &Other) {
    *this = {Int + Other.Int, Str + Other.Str, Dex + Other.Dex,
             Vit + Other.Vit};
    return *this;
  }
};

inline StatValues operator+(const StatValues &Lhs,
                            const StatValues &Rhs) noexcept {
  auto Copy = Lhs;
  Copy += Rhs;
  return Copy;
}

struct StatsComp {
  StatValues Base;
  StatValues Bonus;

  inline void reset() { Bonus = {0, 0, 0, 0}; }
  inline void add(StatValues Bonus) { this->Bonus += Bonus; }
  inline StatValues effective() const { return Base + Bonus; }
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
};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_STATS_H