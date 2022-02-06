#include <cxxg/Utils.h>

#include <chrono>
#include <fcntl.h>
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

bool setStdinBlocking(bool Enabled) {
  int Flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  if (Enabled) {
    Flags &= ~O_NONBLOCK;
    return fcntl(STDIN_FILENO, F_SETFL, Flags) == 0;
  } else {
    Flags |= O_NONBLOCK;
    return fcntl(STDIN_FILENO, F_SETFL, Flags) == 0;
  }
}

int getCharNonBlock() {
  setStdinBlocking(false);
  unsigned char Char;
  if (read(STDIN_FILENO, &Char, sizeof(Char)) < 0) {
    return EOF;
  }
  return Char;
}

// FIXME this aligns non-blocking with blocking getchar, but parsing
// escape sequences
// up - "\033[A"
// down - "\033[B"
// left - "\033[D"
// right - "\033[C"
int getCharNonBlockHandleEscape() {
  int Char = getCharNonBlock();
  if (Char != KEY_ESC) {
    return Char;
  }
  Char = getCharNonBlock();
  if (Char != static_cast<int>('[')) {
    return KEY_INVALID;
  }
  Char = getCharNonBlock();
  switch (Char) {
  case KEY_UP:
  case KEY_DOWN:
  case KEY_LEFT:
  case KEY_RIGHT:
    return Char;
  default:
    break;
  }
  return KEY_INVALID;
}

} // namespace

void registerSigintHandler(::std::function<void()> const &Handler) {
  SigintHandler = Handler;
  signal(SIGINT, handleSigint);
}

int getChar(bool Blocking) {
  if (Blocking) {
    return getchar();
  }
  return getCharNonBlockHandleEscape();
}

void clearStdin() {
  while(getchar() != EOF) {}
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
