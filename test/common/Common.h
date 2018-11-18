#ifndef COMMON_H
#define COMMON_H

#include <cxxg/Screen.h>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <vector>

namespace {

inline ::std::ostream &operator<<(::std::ostream &Out, ::cxxg::Color Cl) {
  Out << Cl.Value;
  return Out;
}

template <typename T>
inline ::std::ostream &operator<<(::std::ostream &Out,
                                  ::std::vector<T> const &Elems) {
  auto Pred = "";
  Out << "{ ";
  for (auto Elem : Elems) {
    Out << Pred << Elem;
    Pred = ", ";
  }
  Out << " }";
  return Out;
}

} // namespace

#endif // #ifndef COMMON_H
