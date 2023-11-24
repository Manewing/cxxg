#include <rogue/UI/Controls.h>
#include <sstream>

namespace rogue::ui {

namespace {

const char *getCharRepr(int Value) {
  switch (Value) {
  case cxxg::utils::KEY_UP:
    return "^";
  case cxxg::utils::KEY_DOWN:
    return "v";
  case cxxg::utils::KEY_LEFT:
    return "<";
  case cxxg::utils::KEY_RIGHT:
    return ">";
  default:
    break;
  }

  if (Value < 33) {
    static const char *AsciiRepr[33] = {
        "NULL",  "SOH",   "STX",   "ETX",   "EOT",   "ENQ",   "ACK",
        "'\\a'", "'\\b'", "'\\t'", "'\\n'", "'\\v'", "'\\f'", "'\\r'",
        "SO",    "SI",    "DLE",   "DC1",   "DC2",   "DC3",   "DC4",
        "NAK",   "SYN",   "ETB",   "CAN",   "EM",    "SUB",   "ESC",
        "FS",    "GS",    "RS",    "US",    "''"};
    return AsciiRepr[Value];
  }
  if (Value < 127) {
    static char Buf[] = "c";
    Buf[0] = Value;
    return Buf;
  }
  if (Value == 127) {
    return "DEL";
  }
  return ".";
}

} // namespace

std::string KeyOption::getInteractMsg(const std::vector<KeyOption> &Options) {
  std::stringstream SS;
  // Assemble the key board inputs first
  const char *Pred = "[";
  for (const auto &Opt : Options) {
    SS << Pred << getCharRepr(Opt.Char);
    Pred = "/";
  }
  SS << "] ";
  // Then the help text
  Pred = "";
  for (const auto &Opt : Options) {
    if (!Opt.Help.empty()) {
      SS << Pred << Opt.Help;
      Pred = "/";
    }
  }
  return SS.str();
}

int Controls::getRemappedChar(int Char) {
  switch (Char) {
  case cxxg::utils::KEY_ENTER:
    return 'e';
  case cxxg::utils::KEY_TAB:
    return Controls::NextWindow.Char;
  default:
    break;
  }
  return Char;
}

} // namespace rogue::ui