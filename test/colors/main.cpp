#include <cxxg/Screen.h>
#include <unistd.h>

#include <sstream>

int main() {
  ::cxxg::Screen Screen(80, 24);

  for (int l = 0; l < 10; l++) {
    Screen[11][34] << ::cxxg::ColorInfo::ColorCode((l+1)%5) << "Hello World!";
    Screen[11][0] << ::cxxg::ColorInfo::ColorCode((l+2)%5) << "Test";
    Screen[11][76] << ::cxxg::ColorInfo::ColorCode((l+3)%5) << "Test";
    Screen[11][10] << "XXX";
    Screen[11][67] << "XXX";
    Screen.update();
    usleep(1000000);
  }

  return 0;
}
