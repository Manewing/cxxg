#include <cxxg/Utils.h>

#include <chrono>
#include <signal.h>

namespace cxxg::utils {

namespace {

::std::function<void()> SigintHandler;

void handleSigint(int) { SigintHandler(); }

} // namespace

std::string getCharTxt(int Char) {
  if (32 <= Char && Char < 127) {
    std::string Str = "' '";
    Str[1] = Char;
    return Str;
  }
  switch (Char) {
  case KEY_NULL:
    return "KEY_NUL";
  case KEY_INVALID:
    return "KEY_INVALID";
  case KEY_TAB:
    return "KEY_TAB";
  case KEY_ENTER:
    return "KEY_ENTER";
  case KEY_ESC:
    return "KEY_ESC";
  case KEY_SPACE:
    return "KEY_SPACE";
  case KEY_UP:
    return "KEY_UP";
  case KEY_DOWN:
    return "KEY_DOWN";
  case KEY_LEFT:
    return "KEY_LEFT";
  case KEY_RIGHT:
    return "KEY_RIGHT";
  case KEY_DEL:
    return "KEY_DEL";
  default:
    break;
  }
  return "???";
}

void registerSigintHandler(::std::function<void()> const &Handler) {
  SigintHandler = Handler;
  signal(SIGINT, handleSigint);
}

std::time_t getTimeStamp() {
  auto Now = std::chrono::system_clock::now();
  return std::chrono::system_clock::to_time_t(Now);
}

} // namespace cxxg::utils
