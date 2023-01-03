#ifndef ROGUE_COMPONENTS_STATS_H
#define ROGUE_COMPONENTS_STATS_H

#include <algorithm>

struct HealthComp {
  unsigned Health = 100;
  unsigned MaxHealth = 100;
};

struct AgilityComp {
  unsigned Agility = 30;
  unsigned AP = 0;
  unsigned MaxAP = 20;

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
  unsigned Damage = 10;
  unsigned APCost = 5;
};

#endif // #ifndef ROGUE_COMPONENTS_STATS_H