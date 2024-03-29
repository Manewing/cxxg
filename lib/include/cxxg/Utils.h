#ifndef CXXG_UTILS_HH
#define CXXG_UTILS_HH

#include <ctime>
#include <cxxg/Types.h>
#include <filesystem>
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

constexpr int KEY_INVALID = -1;
constexpr int KEY_NULL = 0;
constexpr int KEY_TAB = 9;
constexpr int KEY_ENTER = 10;
constexpr int KEY_ESC = 27;
constexpr int KEY_SPACE = 32;
constexpr int KEY_DEL = 127;
constexpr int KEY_UP = 1001;
constexpr int KEY_DOWN = 1002;
constexpr int KEY_RIGHT = 1003;
constexpr int KEY_LEFT = 1004;
constexpr int KEY_DEL_C = 1005;

/// Deals with the setup of the terminal screen
void setupTerminal();

/// @brief Returns a string representation of the given character, in alignment
/// with the constants listed above
/// @param Char The character to get the text for
std::string getCharTxt(int Char);

/// Helper function for registering a handler (e.g. a lambda) for
/// a SIGINT signal (after Ctrl+C). Note that only one handler can
/// be registered.
/// @param[in] Handler - Handler to register
void registerSigintHandler(::std::function<void()> const &Handler);

/// Helper function for registering a handler (e.g. a lambda) for
/// a SIGWINCH signal (after terminal resize). Note that only one handler can
/// be registered.
/// @param[in] Handler - Handler to register
void registerWindowResizeHandler(
    ::std::function<void(cxxg::types::Size)> const &Handler);

/// Helper function to check for keyboard input, returning single character.
/// @param[in] Blocking If true waits until key is pressed
int getChar(bool Blocking = true);

/// Clears the current stdin buffer
void clearStdin();

/// Returns true if the terminal input is buffered or not. Buffered means
/// that a '\n' is needed after input.
bool hasBufferedInput();

/// Switches the input from buffered ('\n' needed after input) to
/// un-buffered (no '\n' needed for character input) or vice versa.
void switchBufferedInput();

/// Returns the path to the home directory if it could be acquired or
/// empty string if not.
std::filesystem::path getHomeDir();

/// Returns the time stamp in nano seconds since the start of the epoch.
std::time_t getTimeStamp();

/// Sleeps for the given amount of micro-seconds.
void sleep(size_t MicroSeconds);

/// Returns the current terminal size
cxxg::types::Size getTerminalSize();

} // namespace utils

} // namespace cxxg

#endif // #ifndef CXXG_UTILS_HH
