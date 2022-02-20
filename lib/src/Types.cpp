#include <cxxg/Types.h>

#include <iostream>

namespace cxxg {
namespace types {

std::ostream &operator<<(std::ostream &Out, const NoColor & /*NC*/) {
  return Out;
}

std::ostream &operator<<(std::ostream &Out, const DefaultColor & /*DC*/) {
  Out << "\033[0m";
  return Out;
}

RgbColor RgbColor::getHeatMapColor(float Min, float Max, float Value) {
  Value = std::max(Min, std::min(Value, Max));
  float Ratio = 2 * (Value - Min) / (Max - Min);
  int R = std::max(0, static_cast<int>(255 * (1 - Ratio)));
  int B = std::max(0, static_cast<int>(255 * (Ratio - 1)));
  int G = 255 - B - R;
  return {static_cast<uint8_t>(R), static_cast<uint8_t>(G),
          static_cast<uint8_t>(B)};
}

std::ostream &operator<<(std::ostream &Out, const RgbColor &Color) {
  // FIXME check if we have a tty, disable if we don't have a terminal
  if (Color.HasBackground) {
    Out << "\033[48;2;" << static_cast<int>(Color.BgR) << ";"
        << static_cast<int>(Color.BgG) << ";" << static_cast<int>(Color.BgB)
        << "m";
  } else {
    Out << DefaultColor{}; // clear everything, include background
  }
  Out << "\033[38;2;" << static_cast<int>(Color.R) << ";"
      << static_cast<int>(Color.G) << ";" << static_cast<int>(Color.B) << "m";
  return Out;
}

std::ostream &operator<<(std::ostream &Out, const TermColor &TC) {
  std::visit(
      Overloaded{
          [&Out](const auto &C) { Out << C; },
      },
      TC);
  return Out;
}

ColoredChar &ColoredChar::operator=(char Char) {
  this->Char = Char;
  return *this;
}

ColoredChar &ColoredChar::operator=(const TermColor &Color) {
  this->Color = Color;
  return *this;
}

std::ostream &operator<<(std::ostream &Out, const ColoredChar &Char) {
  Out << Char.Color << Char.Char;
  if (!std::holds_alternative<NoColor>(Char.Color)) {
    Out << DefaultColor{};
  }
  return Out;
}

}; // namespace types
}; // namespace cxxg
