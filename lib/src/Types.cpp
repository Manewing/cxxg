#include <cxxg/Types.h>

#include <iostream>

namespace cxxg {
namespace types {

Color Color::NONE = {0};
Color Color::RED = {31};
Color Color::GREEN = {32};
Color Color::YELLOW = {33};
Color Color::BLUE = {34};
Color Color::GREY = {90};

}; // namespace types
}; // namespace cxxg

::std::ostream &operator<<(::std::ostream &Out, ::cxxg::types::Color const Cl) {
  Out << "\033[" << Cl.Value << "m";
  return Out;
}
