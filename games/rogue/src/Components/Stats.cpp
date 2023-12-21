#include <iostream>
#include <rogue/Components/Stats.h>

namespace rogue {

std::ostream &StatPoints::dump(std::ostream &Out, bool DumpZero = false) const {
  const char *Pred = "";
  bool AnyDumped = false;
  if (Int != 0 || DumpZero) {
    Out << Pred << "Int: " << Int;
    Pred = ", ";
    AnyDumped = true;
  }
  if (Str != 0 || DumpZero) {
    Out << Pred << "Str: " << Str;
    Pred = ", ";
    AnyDumped = true;
  }
  if (Dex != 0 || DumpZero) {
    Out << Pred << "Dex: " << Dex;
    Pred = ", ";
    AnyDumped = true;
  }
  if (Vit != 0 || DumpZero) {
    Out << Pred << "Vit: " << Vit;
    Pred = ", ";
    AnyDumped = true;
  }
  if (!AnyDumped) {
    Out << "Nothing";
  }
  return Out;
}

std::ostream &operator<<(std::ostream &Out, const StatPoints &SP) {
  return SP.dump(Out, /*DumpZero=*/true);
}

StatValue ValueRegenCompBase::restore(StatValue Amount) {
  Value = std::min(Value + Amount, MaxValue);
  return Value;
}

StatValue ValueRegenCompBase::reduce(StatValue Amount) {
  Value = std::max(Value - Amount, 0.0);
  return Value;
}

bool ValueRegenCompBase::hasAmount(StatValue Amount) {
  return Value >= Amount;
}

bool ValueRegenCompBase::tryReduce(StatValue Amount) {
  if (hasAmount(Amount)) {
    reduce(Amount);
    return true;
  }
  return false;
}

} // namespace rogue