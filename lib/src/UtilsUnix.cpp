#include <cxxg/Utils.h>

#include <chrono>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

namespace cxxg {

namespace utils {

namespace {

bool BufferedInputEnabled = true;
termios TermAttrOld;

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

int readChar() {
  unsigned char Char;
  if (read(STDIN_FILENO, &Char, sizeof(Char)) < 0) {
    return EOF;
  }
  return Char;
}

// up - "\033[A"
// down - "\033[B"
// left - "\033[D"
// right - "\033[C"
int decodeEscape(int CharA, int CharB) {
  if (CharA != static_cast<int>('[')) {
    return KEY_ESC;
  }
  switch (CharB) {
  case 'A':
    return KEY_UP;
  case 'B':
    return KEY_DOWN;
  case 'D':
    return KEY_LEFT;
  case 'C':
    return KEY_RIGHT;
  case '3':
    readChar();
    return KEY_DEL_C;
  default:
    return CharB;
  }
  return KEY_INVALID;
}

int getCharNonBlockHandleEscape() {
  int Char = readChar();
  if (Char != KEY_ESC) {
    return Char;
  }
  int CharA = readChar();
  int CharB = readChar();
  return decodeEscape(CharA, CharB);
}

int getCharBlockHandleEscape() {
  int Char = readChar();
  if (Char != KEY_ESC) {
    return Char;
  }
  setStdinBlocking(false);
  int CharA = readChar();
  int CharB = readChar();
  setStdinBlocking(true);
  return decodeEscape(CharA, CharB);
}

} // namespace

void setupTerminal() {}

int getChar(bool Blocking) {
  if (Blocking) {
    return getCharBlockHandleEscape();
  }
  setStdinBlocking(false);
  int Char = getCharNonBlockHandleEscape();
  clearStdin();
  setStdinBlocking(true);
  return Char;
}

void clearStdin() {
  while (getchar() != EOF) {
  }
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

std::filesystem::path getHomeDir() {
  if (auto HomeDir = getenv("HOME")) {
    return HomeDir;
  }
  return "";
}

void sleep(size_t MicroSeconds) { usleep(MicroSeconds); }

cxxg::types::Size getTerminalSize() {
  winsize Ws;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &Ws);
  if (!Ws.ws_col || !Ws.ws_row) {
    return {80, 24};
  }
  return {Ws.ws_col, Ws.ws_row};
}

} // namespace utils

} // namespace cxxg