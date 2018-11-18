#include "MasterMind.h"

#include <algorithm>
#include <cxxg/Utils.h>
#include <iomanip>
#include <stdexcept>

::std::map<char, ::cxxg::Color> MasterMind::Colors = {
    {' ', ::cxxg::Color::NONE},   {'a', ::cxxg::Color::RED},
    {'b', ::cxxg::Color::GREEN},  {'c', ::cxxg::Color::BLUE},
    {'d', ::cxxg::Color::YELLOW}, {'e', ::cxxg::Color::RED},
    {'f', ::cxxg::Color::GREEN},  {'g', ::cxxg::Color::BLUE},
    {'h', ::cxxg::Color::YELLOW}};

::std::string MasterMind::formatTime(size_t NanoSeconds) {
  size_t Seconds = NanoSeconds * 1e-9;
  size_t Minutes = Seconds / 60;
  Seconds -= Minutes * 60;

  ::std::stringstream SS;
  if (Minutes) {
    SS << ::std::setw(2) << Minutes << "m ";
  } else {
    SS << ::std::string(4, ' ');
  }
  SS << ::std::setw(2) << Seconds << "s";

  return SS.str();
}

MasterMind::MasterMind()
    : State(Running), InputPosition(0), Code(getRandomCode(4, 8)) {
  CurrentGuess.resize(4, ' ');
  GameStartTimeStamp = ::cxxg::utils::getTimeStamp();
}

MasterMind::~MasterMind() {
  handleExit();
}

void MasterMind::initialize(bool BufferedInput) {
  ::cxxg::Game::initialize(BufferedInput);

  drawLogoAnimation();

  handleDraw();
}

void MasterMind::handleInput(int Char) {
  switch (Char) {
  case 68: // left
    handleLeft();
    break;
  case 67: // right
    handleRight();
    break;
  case '\n':
    handleReturn();
    break;
  case 'q':
    handleGameOver();
    break;
  default:
    if ('a' <= Char && Char <= 'h') {
      CurrentGuess.at(InputPosition) = Char;
      handleRight();
    }
    break;
  }
}

void MasterMind::handleReturn() {
  if (!isValidGuess(CurrentGuess)) {
    warn() << "Invalid guess, no spaces or duplicates allowed";
    return;
  }

  // reset input position
  InputPosition = 0;

  // add guess to history
  size_t Blacks = getBlacks(Code, CurrentGuess);
  size_t Whites = getWhites(Code, CurrentGuess);
  size_t TimeStamp = ::cxxg::utils::getTimeStamp() - GameStartTimeStamp;
  HistGuess HG{CurrentGuess, Blacks, Whites, TimeStamp};
  History.push_back(HG);

  // check if last guess was correct
  if (History.back().Blacks == 4) {
    State = Victory;
    handleGameOver();
  }

  // check if was last guess
  if (History.size() >= 10) {
    handleGameOver();
  }

  // clear current guess
  ::std::fill(CurrentGuess.begin(), CurrentGuess.end(), ' ');
}

void MasterMind::handleLeft() {
  InputPosition--;
  if (InputPosition < 0) {
    InputPosition = CurrentGuess.size() - 1;
  }
}

void MasterMind::handleRight() {
  InputPosition++;
  if (InputPosition >= static_cast<int>(CurrentGuess.size())) {
    InputPosition = 0;
  }
}

void MasterMind::handleDraw() {
  // define game size
  ::cxxg::ScreenSize GameSize{22, 29};

  // get the offsets
  ::cxxg::ScreenSize const Offset = getOffset(GameSize);

  // draw the board outline
  int Y = Offset.Y;

  ::cxxg::Color CodeColor = ::cxxg::Color::NONE;
  if (State == GameOver) {
    CodeColor = ::cxxg::Color::RED;
  } else if (State == Victory) {
    CodeColor = ::cxxg::Color::GREEN;
  }
  Scr[Y++][Offset.X] << CodeColor << "+---+---+---+---+";
  Scr[Y][Offset.X] << CodeColor << "|   |   |   |   |";
  Scr[Y++].setColor(Offset.X + 1, Offset.X + 15, ::cxxg::Color::GREY);
  Scr[Y++][Offset.X] << CodeColor << "+---+---+---+---+";

  // one line gap
  Y += 1;

  for (size_t L = 0; L < 10; L++) {
    Scr[Y++][Offset.X] << "+---+---+---+---+  +-+-+-+-+";
    Scr[Y][Offset.X] << "|   |   |   |   |  | | | | |";
    Scr[Y].setColor(Offset.X + 1, Offset.X + 15, ::cxxg::Color::GREY);
    Scr[Y++].setColor(Offset.X + 20, Offset.X + 27, ::cxxg::Color::GREY);
  }
  Scr[Y++][Offset.X] << "+---+---+---+---+  +-+-+-+-+";

  // one line gap
  Y += 1;

  Scr[Y++][Offset.X] << "+---+---+---+---+";
  Scr[Y][Offset.X] << "|   |   |   |   |";
  Scr[Y++].setColor(Offset.X + 1, Offset.X + 15, ::cxxg::Color::GREY);
  Scr[Y][Offset.X] << "+---+---+---+---+";

  // create lambda for drawing guess at given Y position
  auto drawGuess = [&](Pins const &Guess, size_t Y) {
    for (int X = Offset.X + 2, L = 0; X < Offset.X + 16; X += 4) {
      Scr[Y][X] = Guess.at(L);
      Scr[Y][X] = Colors.at(Guess.at(L++));
    }
  };

  // draw game history
  Y = Y - 5;
  for (auto const &Elem : History) {
    // draw old guess
    drawGuess(Elem.Guess, Y);

    // draw black pins
    int X = Offset.X + 20;
    int EndX = Offset.X + 20 + Elem.Blacks * 2;
    for (; X < EndX; X += 2) {
      Scr[Y][X] = '#';
      Scr[Y][X] = ::cxxg::Color::NONE;
    }

    // draw white pins
    X = Offset.X + 20 + Elem.Blacks * 2;
    EndX = Offset.X + 20 + Elem.Blacks * 2 + Elem.Whites * 2;
    for (; X < EndX; X += 2) {
      Scr[Y][X] = '+';
      Scr[Y][X] = ::cxxg::Color::NONE;
    }

    // FIXME this will be clipped of if screen is to small
    auto TimeStampStr = formatTime(Elem.TimeStamp);
    Scr[Y][Offset.X - TimeStampStr.size() - 1] << TimeStampStr;

    Y -= 2;
  }

  // draw game input
  ::cxxg::ScreenSize const InputOffset = {Offset.X, Offset.Y + GameSize.Y - 2};

  // draw current guess
  drawGuess(CurrentGuess, InputOffset.Y);

  // highlight input position, TODO cleanup
  int InputStartX = 1 + Offset.X + InputPosition * 4;
  int InputEndX = 4 + Offset.X + InputPosition * 4;
  ::cxxg::Color const HlCl{7};
  Scr[InputOffset.Y][InputStartX - 1] = ::cxxg::Color::NONE;
  Scr[InputOffset.Y].setColor(InputStartX, InputEndX, HlCl);
  Scr[InputOffset.Y][InputEndX] = ::cxxg::Color::NONE;

  // draw current code
  ::cxxg::ScreenSize const CodeOffset = {Offset.X, Offset.Y + 1};

  for (int L = 0; L < 4; L++) {
    int X = 2 + Offset.X + L * 4;
    Scr[CodeOffset.Y][X] = State == Running ? '?' : Code.at(L);
    Scr[CodeOffset.Y][X] =
        State == Running ? ::cxxg::Color::GREY : Colors.at(Code.at(L));
  }

  if (State == GameOver) {
    Scr[Offset.Y + GameSize.Y / 2][Offset.X + 4] << " FAILURE ";
  } else if (State == Victory) {
    Scr[Offset.Y + GameSize.Y / 2][Offset.X + 4] << " CRACKED ";
  }

  ::cxxg::Game::handleDraw();
}

void MasterMind::drawLogoAnimation() {
  ::cxxg::ScreenSize const LogoSize{27, 8};
  auto const Offset = getOffset(LogoSize);

  int Y = 0;
  Scr[Offset.Y + Y++][Offset.X] << "888b     d888 888b     d888";
  Scr[Offset.Y + Y++][Offset.X] << "8888b   d8888 8888b   d8888";
  Scr[Offset.Y + Y++][Offset.X] << "88888b.d88888 88888b.d88888";
  Scr[Offset.Y + Y++][Offset.X] << "888Y88888P888 888Y88888P888";
  Scr[Offset.Y + Y++][Offset.X] << "888 Y888P 888 888 Y888P 888";
  Scr[Offset.Y + Y++][Offset.X] << "888  Y8P  888 888  Y8P  888";
  Scr[Offset.Y + Y++][Offset.X] << "888       888 888       888";
  Scr[Offset.Y + Y++][Offset.X] << "888       888 888       888";
  Scr.setColor(Offset, {Offset.X + LogoSize.X, Offset.Y + LogoSize.Y},
               ::cxxg::Color::GREY);

  for (int X = Offset.X; X < Offset.X + LogoSize.X; X++) {
    Scr.setColor({X, Offset.Y}, {X + 1, Offset.Y + LogoSize.Y},
                 ::cxxg::Color::NONE);
    Scr.update();
    ::cxxg::utils::sleep(30000);
  }

  Scr.setColor(Offset, {Offset.X + LogoSize.X, Offset.Y + LogoSize.Y},
               ::cxxg::Color::NONE);
  Scr.update();
  ::cxxg::utils::sleep(500000);

  Scr.clear();
}

void MasterMind::handleGameOver() {
  State = State == Running ? GameOver : State;
  GameRunning = false;
}

bool MasterMind::hasDuplicates(Pins const &Guess) {
  Pins Unique = Guess;

  // erase duplices
  ::std::sort(Unique.begin(), Unique.end());
  Unique.erase(::std::unique(Unique.begin(), Unique.end()), Unique.end());

  // if size changed we had duplicates
  if (Unique.size() != Guess.size()) {
    return true;
  }

  return false;
}

bool MasterMind::isComplete(Pins const &Guess) {
  auto It = ::std::find(Guess.begin(), Guess.end(), ' ');
  if (It != Guess.end()) {
    return false;
  }

  return true;
}

bool MasterMind::isValidGuess(Pins const &Guess) {
  return not hasDuplicates(Guess) and isComplete(Guess);
}

size_t MasterMind::getBlacks(Pins const &Code, Pins const &Guess) {
  if (Code.size() != Guess.size()) {
    THROW_CXXG_ERROR("MasterMind: Code size (" << Code.size() << ") does not"
                                               << " match Guess size ("
                                               << Guess.size() << ")");
  }

  size_t Count = 0;

  for (size_t L = 0; L < Code.size(); L++) {
    if (Guess.at(L) == Code.at(L)) {
      Count++;
    }
  }

  return Count;
}

size_t MasterMind::getWhites(Pins const &Code, Pins const &Guess) {
  if (Code.size() != Guess.size()) {
    THROW_CXXG_ERROR("MasterMind: Code size (" << Code.size() << ") does not"
                                               << " match Guess size ("
                                               << Guess.size() << ")");
  }

  size_t Count = 0;

  for (auto Elem : Guess) {
    auto It = ::std::find(Code.begin(), Code.end(), Elem);
    if (It != Code.end()) {
      Count++;
    }
  }
  return Count - getBlacks(Code, Guess);
}

MasterMind::Pins MasterMind::getRandomCode(size_t CodeSize, size_t PinCount) {
  auto const MaxPinCount = static_cast<size_t>('z' - 'a');

  if (PinCount >= MaxPinCount) {
    THROW_CXXG_ERROR("MasterMind: Cannot create code, pin count "
                     << "(" << PinCount << ") must be smaller than ("
                     << MaxPinCount << ")");
  }

  if (CodeSize > PinCount) {
    THROW_CXXG_ERROR("MasterMind: Cannot create code ("
                     << CodeSize << ") which is greater than pin count ("
                     << PinCount << ")");
  }

  Pins Code;
  Code.resize(PinCount, 'a');

  for (auto &Elem : Code) {
    Elem += static_cast<Pin>(PinCount-- % MaxPinCount);
  }

  ::std::shuffle(Code.begin(), Code.end(), RndEngine);
  Code.erase(Code.begin() + CodeSize, Code.end());

  return Code;
}
