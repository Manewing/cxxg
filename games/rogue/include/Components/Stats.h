#ifndef ROGUE_COMPONENTS_STATS_H
#define ROGUE_COMPONENTS_STATS_H

#include <algorithm>

using StatValue = double;

struct HealthComp {
  StatValue Value = 100;
  StatValue MaxValue = 100;

  unsigned TickPeriod = 2;
  unsigned TicksLeft = 1;
  StatValue RegenAmount = 0.5;
};

struct ManaComp {
  StatValue Value = 100;
  StatValue MaxValue = 100;

  unsigned TickPeriod = 4;
  unsigned TicksLeft = 1;
  StatValue RegenAmount = 0.1;
};

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

#endif // #ifndef ROGUE_COMPONENTS_STATS_H