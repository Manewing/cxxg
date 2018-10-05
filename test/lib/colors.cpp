#include "Common.h"
#include <cxxg/Screen.h>
#include <unistd.h>

// TODO Currently this is only a manual test, but it should be an automatic
//      test.
int main() {
  ::cxxg::Screen Screen(::cxxg::ScreenSize{80, 24});

  for (int l = 0; l < 10; l++) {
    Screen[11][34] << ::cxxg::Color{l % 4 + 31} << "Hello World!";
    Screen[11][0] << ::cxxg::Color{(l + 1) % 4 + 31} << "Test";
    Screen[11][76] << ::cxxg::Color{(l + 2) % 4 + 31} << "Test";
    Screen[11][10] << "XXX";
    Screen[11][67] << "XXX";
    Screen.update();
    usleep(10000);
  }

  RETURN_SUCCESS;
}
