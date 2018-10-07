#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <sstream>
#include <vector>

// TODO Replace this file with google test....

bool Success = true;

#define EXPECT_EQ(a, b)                                                        \
  if (!((a) == (b))) {                                                         \
    ::std::cerr << "ERROR: expected '" << #a << "' = '" << a                   \
                << "' to be equal to '" << #b << "' = '" << b << "'"           \
                << ::std::endl;                                                \
    Success = false;                                                           \
  }
#define EXPECT_EQ_MSG(a, b, msg)                                               \
  if (!((a) == (b))) {                                                         \
    ::std::cerr << "ERROR[" << msg << "]: expected '" << #a << "' = '" << a    \
                << "' to be equal to '" << #b << "' = '" << b << "'"           \
                << ::std::endl;                                                \
    Success = false;                                                           \
  }

#define RETURN_SUCCESS return Success ? 0 : 1;

template <typename T>
bool operator==(::std::vector<T> const &A, ::std::vector<T> const &B) {
  if (A.size() != B.size())
    return false;

  for (size_t X = 0; X < A.size(); X++) {
    if (A.at(X) != B.at(X))
      return false;
  }

  return true;
}

template <typename T>
::std::ostream &operator<<(::std::ostream &Out,
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

#endif // #ifndef COMMON_H
