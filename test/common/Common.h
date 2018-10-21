#ifndef COMMON_H
#define COMMON_H

#include <cxxg/Screen.h>
#include <iostream>
#include <sstream>
#include <vector>

// TODO Replace this file with google test....

bool Success = true;

#define EXPECT_EQ_BASE(a, b, msg)                                              \
  if (!((a) == (b))) {                                                         \
    ::std::cerr << "ERROR" << msg << ": expected '" << #a << "' = '" << a      \
                << "' to be equal to '" << #b << "' = '" << b << "'"           \
                << ::std::endl;                                                \
    Success = false;                                                           \
  }
#define EXPECT_EQ(a, b) EXPECT_EQ_BASE(a, b, "")
#define EXPECT_EQ_MSG(a, b, msg) EXPECT_EQ_BASE(a, b, "[" << msg << "]")

#define EXPECT_THROW_BASE(stmt, except, msg)                                   \
  try {                                                                        \
    stmt;                                                                      \
    ::std::cerr << "ERROR" << msg << ": expected '" << #stmt                   \
                << "' to throw an exception" << ::std::endl;                   \
    Success = false;                                                           \
  } catch (except const &E) {                                                  \
    ::std::cerr << "INFO: " << E.what() << ::std::endl;                        \
  }
#define EXPECT_THROW(stmt, except) EXPECT_THROW_BASE(stmt, except, "")
#define EXPECT_THROW_MSG(stmt, except, msg)                                    \
  EXPECT_THROW_BASE(stmt, except, "[" << msg << "]")

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

::std::ostream &operator<<(::std::ostream &Out, ::cxxg::Color Cl) {
  Out << Cl.Value;
  return Out;
}

template <typename T>
::std::ostream &operator<<(::std::ostream &Out, ::std::vector<T> const &Elems) {
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
