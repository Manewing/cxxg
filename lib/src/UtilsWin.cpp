#include <cxxg/Utils.h>
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>

namespace cxxg::utils {

namespace {

bool BufferedInputEnabled = true;
DWORD ConsoleModeOld = 0;

int getCharNonBlocking() {
  INPUT_RECORD InputRecord = {0};
  DWORD NumRead = 0;

  int Char = KEY_INVALID;
  auto Handle = GetStdHandle(STD_INPUT_HANDLE);
  if (Handle == INVALID_HANDLE_VALUE) {
    return KEY_INVALID;
  }

  if (GetNumberOfConsoleInputEvents(Handle, &NumRead) == FALSE ||
      NumRead == 0) {
    return KEY_INVALID;
  }

  if (ReadConsoleInput(Handle, &InputRecord, 1, &NumRead) == FALSE) {
    return KEY_INVALID;
  }
  if (InputRecord.EventType != KEY_EVENT ||
      InputRecord.Event.KeyEvent.bKeyDown == FALSE) {
    return KEY_INVALID;
  }

  if (InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE) {
    return KEY_ESC;
  }
  if (InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_UP) {
    return KEY_UP;
  }
  if (InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_DOWN) {
    return KEY_DOWN;
  }
  if (InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_LEFT) {
    return KEY_LEFT;
  }
  if (InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_RIGHT) {
    return KEY_RIGHT;
  }
  if (InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_DELETE) {
    return KEY_DEL_C;
  }
  if (InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_RETURN) {
    return KEY_ENTER;
  }
  if (InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_BACK) {
    return KEY_DEL;
  }

  if (InputRecord.Event.KeyEvent.uChar.AsciiChar == 0) {
    return KEY_INVALID;
  }
  return InputRecord.Event.KeyEvent.uChar.AsciiChar;
}

int getCharBlocking() {
  // Wait for input
  if (WaitForSingleObject(GetStdHandle(STD_INPUT_HANDLE), INFINITE) !=
      WAIT_OBJECT_0) {
    return KEY_INVALID;
  }
  int Char = getCharNonBlocking();
  while (Char == KEY_INVALID) {
    Char = getCharNonBlocking();
  }
  return Char;
}

} // namespace

int getChar(bool Blocking) {
  if (Blocking) {
    return getCharBlocking();
  }
  return getCharNonBlocking();
}

void clearStdin() {
  while (getchar() != EOF) {
  }
}

bool hasBufferedInput() { return BufferedInputEnabled; }

void switchBufferedInput() {
  if (BufferedInputEnabled) {
    BufferedInputEnabled = false;
    GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &ConsoleModeOld);
    ConsoleModeOld &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ConsoleModeOld);
  } else {
    BufferedInputEnabled = true;
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ConsoleModeOld);
  }
}

std::filesystem::path getHomeDir() {
  if (const char *HomeDir = getenv("USERPROFILE")) {
    return HomeDir;
  }
  return "";
}

void sleep(size_t MicroSeconds) { Sleep(MicroSeconds / 1000 / 1000); }

std::pair<unsigned, unsigned> getTerminalSize() {
  CONSOLE_SCREEN_BUFFER_INFO CSBI;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &CSBI);
  return {CSBI.srWindow.Right - CSBI.srWindow.Left + 1,
          CSBI.srWindow.Bottom - CSBI.srWindow.Top + 1};
}

} // namespace cxxg::utils
