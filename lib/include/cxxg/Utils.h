#ifndef CXXG_UTILS_HH
#define CXXG_UTILS_HH

#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>

#define THROW_CXXG_ERROR(msg)                                                  \
  {                                                                            \
    ::std::stringstream cxxg__SS;                                              \
    cxxg__SS << "cxxg:" << msg;                                                \
    throw ::std::runtime_error(cxxg__SS.str());                                \
  }

namespace cxxg {

namespace utils {

/// Helper function for registering a handler (e.g. a lambda) for
/// a SIGINT signal (after Ctrl+C). Note that only one handler can
/// be registered.
/// @param[in] Handler - Handler to register
void registerSigintHandler(::std::function<void()> const &Handler);

/// Returns true if the terminal input is buffered or not. Buffered means
/// that a Return is needed after input.
bool hasBufferedInput();

/// Switches the input from buffered to un-buffered or vice versa.
void switchBufferedInput();

/// Returns the path to the home directory if it could be acquired or
/// empty string if not.
::std::string getHomeDir();

} // namespace utils

} // namespace cxxg

#endif // #ifndef CXXG_UTILS_HH
