#include <rogue/UI/Controls.h>
#include <sstream>

namespace rogue::ui {

std::string KeyOption::getInteractMsg(const std::vector<KeyOption> &Options) {
  std::stringstream SS;
  // Assemble the key board inputs first
  const char *Pred = "[";
  for (const auto &Opt : Options) {
    SS << Pred << static_cast<char>(Opt.Char);
    Pred = "/";
  }
  SS << "] ";
  // Then the help text
  Pred = "";
  for (const auto &Opt : Options) {
    SS << Pred << Opt.Help;
    Pred = "/";
  }
  return SS.str();
}



}