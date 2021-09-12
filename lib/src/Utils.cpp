#include <cxxg/Utils.h>

#include <chrono>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

namespace cxxg {

namespace utils {

namespace {

bool BufferedInputEnabled = true;
termios TermAttrOld;

::std::function<void()> SigintHandler;

void handleSigint(int) { SigintHandler(); }

} // namespace

void registerSigintHandler(::std::function<void()> const &Handler) {
  SigintHandler = Handler;
  signal(SIGINT, handleSigint);
}

bool hasBufferedInput() {
  // TODO this should not be a flag, this should be read from the
  // attributes
  return BufferedInputEnabled;
}

void switchBufferedInput() {
  if (BufferedInputEnabled) {

    // disable buffered input
    TermAttrOld = termios{};
    tcgetattr(STDIN_FILENO, &TermAttrOld);
    auto TermAttrNew = TermAttrOld;
    TermAttrNew.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &TermAttrNew);

    BufferedInputEnabled = false;

  } else {
    tcsetattr(STDIN_FILENO, TCSANOW, &TermAttrOld);
    BufferedInputEnabled = true;
  }
}

::std::string getHomeDir() {
  if (auto HomeDir = getenv("HOME")) {
    return HomeDir;
  }
  return "";
}

std::time_t getTimeStamp() {
  auto Now = std::chrono::system_clock::now();
  return std::chrono::system_clock::to_time_t(Now);
}

void sleep(size_t MicroSeconds) { usleep(MicroSeconds); }

} // namespace utils

} // namespace cxxg
