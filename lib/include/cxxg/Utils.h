#ifndef CXXG_UTILS_HH
#define CXXG_UTILS_HH

#include <functional>

namespace cxxg {

namespace utils {

void registerSigintHandler(::std::function<void()> const &Handler);

bool hasBufferedInput();
void switchBufferedInput();

} // namespace utils

} // namespace cxxg

#endif // #ifndef CXXG_UTILS_HH
