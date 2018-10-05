#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <sstream>

// TODO Replace this file with google test....

bool Success = true;

#define EXPECT_EQ(a, b)                                                        \
  if (!((a) == (b))) {                                                         \
    ::std::cerr << "ERROR: expected '" << #a << "' = " << a                    \
                << " to be equal to '" << #b << "' = " << b << ::std::endl;    \
    Success = false;                                                           \
  }
#define EXPECT_EQ_MSG(a, b, msg)                                               \
  if (!((a) == (b))) {                                                         \
    ::std::cerr << "ERROR[" << msg << "]: expected '" << #a << "' = " << a     \
                << " to be equal to '" << #b << "' = " << b << ::std::endl;    \
    Success = false;                                                           \
  }

#define RETURN_SUCCESS return Success ? 0 : 1;

#endif // #ifndef COMMON_H
