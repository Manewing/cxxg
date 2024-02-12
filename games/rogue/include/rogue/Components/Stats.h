#ifndef ROGUE_COMPONENTS_STATS_H
#define ROGUE_COMPONENTS_STATS_H

#include <algorithm>
#include <array>
#include <iosfwd>
#include <numeric>
#include <rogue/Types.h>
#include <limits>
#include <tuple>

namespace rogue {

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

  inline constexpr std::array<StatPoint *, 4> all() {
    return {{&Int, &Str, &Dex, &Vit}};
  }

  inline constexpr std::array<StatPoint, 4> all() const {
    return {{Int, Str, Dex, Vit}};
  }

  inline StatPoint sum() const {
    const auto All = all();
    return std::accumulate(All.begin(), All.end(), 0);
  }

  inline auto tie() { return std::tie(Int, Str, Dex, Vit); }
  inline auto tie() const { return std::tie(Int, Str, Dex, Vit); }

  std::ostream &dump(std::ostream &Out, bool DumpZero) const;

  template <class Archive> void serialize(Archive &Ar) { Ar(Int, Str, Dex, Vit); }
};

std::ostream &operator<<(std::ostream &Out, const StatPoints &SP);

inline bool operator==(const StatPoints &Lhs, const StatPoints &Rhs) noexcept {
  return Lhs.tie() == Rhs.tie();
}

inline bool operator!=(const StatPoints &Lhs, const StatPoints &Rhs) noexcept {
  return !(Lhs == Rhs);
}

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
  StatPoints Base = {};
  StatPoints Bonus = {};

  inline void reset() { Bonus = {0, 0, 0, 0}; }
  inline void add(StatPoints Bonus) { this->Bonus += Bonus; }
  inline StatPoints effective() const { return Base + Bonus; }

  template <class Archive> void serialize(Archive &Ar) { Ar(Base, Bonus); }
};

struct ValueRegenCompBase {
  StatValue Value = std::numeric_limits<StatValue>::max();
  StatValue MaxValue = 100;

  unsigned TickPeriod = 4;
  unsigned TicksLeft = 1;
  StatValue RegenAmount = 0.05;

  bool hasAmount(StatValue Value);
  bool tryReduce(StatValue Amount);
  StatValue restore(StatValue Amount);
  StatValue reduce(StatValue Amount);
};

struct HealthComp : public ValueRegenCompBase {
  bool isDead() const { return Value <= 0; }
};

struct ManaComp : public ValueRegenCompBase {};

struct AgilityComp {
  StatValue Agility = 0;
  StatValue AP = 0;

  void gainAP(StatValue APAmount) { AP += APAmount; }

  bool hasEnoughAP(StatValue APAmount) const { return AP >= APAmount; }

  bool trySpendAP(StatValue APAmount) {
    if (!hasEnoughAP(APAmount)) {
      return false;
    }
    spendAP(APAmount);
    return true;
  }

  void spendAP(StatValue APAmount) { AP -= APAmount; }
};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_STATS_H