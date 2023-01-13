#include <rogue/Components/Stats.h>

namespace rogue {

StatValue ValueRegenCompBase::restore(StatValue Amount) {
  Value = std::min(Value + Amount, MaxValue);
  return Value;
}

StatValue ValueRegenCompBase::reduce(StatValue Amount) {
  Value = std::max(Value - Amount, 0.0);
  return Value;
}

}