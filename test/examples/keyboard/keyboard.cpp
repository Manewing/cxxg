#include <array>
#include <cxxg/Game.h>
#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <iomanip>
#include <stdexcept>

struct SwitchBlocking {
  bool Blocking = false;
  unsigned TickDelayUs = 100000;
};

class KeyboardTest : public cxxg::Game {
public:
  static constexpr int CfgElems = 3;
  static constexpr cxxg::types::RgbColor HighlightColor{255, 255, 255, true,
                                                        100, 100, 100};

public:
  explicit KeyboardTest(cxxg::Screen &Scr) : cxxg::Game(Scr) {
    Chars.resize(12);
  }

  void initialize(bool BufferedInput = false, unsigned TickDelayUs = 0) final {
    cxxg::Game::initialize(BufferedInput, TickDelayUs);
    handleDraw();
  }

  void run(bool Blocking = true) final {
    (void)Blocking;
    cxxg::Game::run(this->Blocking);
  }

  bool handleInput(int Char) final {
    std::rotate(Chars.rbegin(), Chars.rbegin() + 1, Chars.rend());
    Chars[0] = Char;

    if (Char == cxxg::utils::KEY_SPACE) {
      ConfigMode = !ConfigMode;
    }

    if (!ConfigMode) {
      return true;
    }

    switch (Char) {
    case cxxg::utils::KEY_SPACE:
      break;
    case cxxg::utils::KEY_LEFT:
      CfgElem = std::max(0, CfgElem - 1);
      break;
    case cxxg::utils::KEY_RIGHT:
      CfgElem = std::min(CfgElems - 1, CfgElem + 1);
      break;
    case cxxg::utils::KEY_ENTER:
      handleConfigChange();
      break;
    case cxxg::utils::KEY_UP:
      if (CfgElem == 2) {
        TickDelayUs += 10000;
      }
      break;
    case cxxg::utils::KEY_DOWN:
      if (CfgElem == 2) {
        TickDelayUs -= 10000;
      }
      break;
    default:
      break;
    }

    return true;
  }

  void handleConfigChange() {
    ConfigMode = false;
    switch (CfgElem) {
    case 0:
      cxxg::utils::switchBufferedInput();
      break;
    case 1:
      throw SwitchBlocking{!Blocking, Blocking ? 100000U : 0U};
      break;
    default:
      break;
    }
  }

  void handleDraw() final {
    std::array<cxxg::types::TermColor, CfgElems> Clrs;
    Clrs.fill(cxxg::types::Color::NONE);

    if (ConfigMode) {
      Clrs[CfgElem] = HighlightColor;
    }

    Scr[0][0] << "[INPUT]: " << Clrs[0]
              << (cxxg::utils::hasBufferedInput() ? "BUF" : "UBUF")
              << cxxg::types::Color::NONE << "|" << Clrs[1]
              << (Blocking ? "BLOCK" : "UBLOCK") << cxxg::types::Color::NONE
              << "   [TICK]: " << Clrs[2] << int(TickDelayUs / 1000.0) << "ms";

    cxxg::types::Size const GameSize{30, Chars.size()};
    auto const Offset = getOffset(GameSize);

    for (std::size_t Pos = 0; Pos < Chars.size(); Pos++) {
      auto YPos = Offset.Y + Chars.size() - Pos;

      uint8_t Scale = 255 - Pos * 12;
      auto Color = cxxg::types::RgbColor({Scale, Scale, Scale});
      Scr[YPos][Offset.X] << Color << "Key pressed: " << std::setw(4)
                          << Chars.at(Pos) << " "
                          << cxxg::utils::getCharTxt(Chars.at(Pos));
    }

    cxxg::Game::handleDraw();
  }

  void setBlocking(bool Blocking) { this->Blocking = Blocking; }

  void setTickDelayUs(unsigned TickDelayUs) { this->TickDelayUs = TickDelayUs; }

private:
  bool ConfigMode = false;
  int CfgElem = 0;
  bool Blocking = true;
  std::vector<int> Chars;
};

void runKeyboardTest(cxxg::Screen &Scr) {
  KeyboardTest KT(Scr);
  KT.initialize();

  while (true) {
    try {
      KT.run();
    } catch (const SwitchBlocking &SB) {
      KT.setBlocking(SB.Blocking);
      KT.setTickDelayUs(SB.TickDelayUs);
      KT.handleDraw();
    }
  }
}

int main() {
  cxxg::Screen Scr(cxxg::Screen::getTerminalSize());
  cxxg::utils::registerSigintHandler([]() { exit(0); });

  try {
    runKeyboardTest(Scr);
  } catch (std::exception const &E) {
    std::cerr << "ERROR: " << E.what() << std::endl;
    return 1;
  }

  return 0;
}