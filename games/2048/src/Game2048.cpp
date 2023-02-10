#include "Game2048.h"

#include <algorithm>
#include <cxxg/Utils.h>
#include <fstream>

Game2048::Game2048(cxxg::Screen &Scr, std::string const &HighScoreFile)
    : cxxg::Game(Scr), Score(0), HighScore(0), State(NewGame),
      HighScoreFile(HighScoreFile) {
  Board.resize(4, ::std::vector<unsigned>{0, 0, 0, 0});

  if (!HighScoreFile.empty()) {
    ::std::ifstream File(HighScoreFile.c_str(), ::std::ios::in);
    if (File.is_open()) {
      File >> HighScore;
    } else {
      warn() << "Could not open highscore file: '" << HighScoreFile
             << "' for reading.";
    }
  }
}

Game2048::~Game2048() { handleExit(); }

void Game2048::initialize(bool BufferedInput, unsigned TickDelayUs) {
  ::cxxg::Game::initialize(BufferedInput, TickDelayUs);

  // draw some nice animation
  drawLogoAnimation();

  // add first two initial elements to the board
  addNewElement();
  addNewElement();

  handleDraw();
}

bool Game2048::handleInput(int Char) {
  if (Char == -1) {
    handleGameOver();
  }

  bool HasMoved = true;
  switch (Char) {
  case cxxg::utils::KEY_LEFT:
    Score += moveLeft(Board);
    break;
  case cxxg::utils::KEY_RIGHT:
    Score += moveRight(Board);
    break;
  case cxxg::utils::KEY_UP:
    Score += moveUp(Board);
    break;
  case cxxg::utils::KEY_DOWN:
    Score += moveDown(Board);
    break;
  case 'q':
    handleGameOver();
    HasMoved = false;
    break;
  default:
    HasMoved = false;
    return false;
  }

  if (HasMoved) {
    addNewElement();
  }

  return true;
}

void Game2048::handleDraw() {
  // define game size
  ::cxxg::types::Size const GameSize{22, 12};

  // get game offset
  auto const Offset = getOffset(GameSize);

  // get message offsets
  auto const MsgOffset = Offset + ::cxxg::types::Size{5, 7};

  // draw the score board
  Scr[Offset] << ::cxxg::types::Color::BLUE << "Score: " << Score;
  Scr[Offset.Y + 1][Offset.X] << ::cxxg::types::Color::GREEN
                              << "HighScore: " << HighScore;

  // draw the board outline
  for (size_t Y = 0; Y < 8; Y += 2) {
    Scr[Offset.Y + 3 + Y][Offset.X] << "+----+----+----+----+";
    Scr[Offset.Y + 4 + Y][Offset.X] << "|    |    |    |    |";
  }
  Scr[Offset.Y + 11][Offset.X] << "+----+----+----+----+";

  // fill the board
  for (size_t X = 0; X < 4; X++) {
    for (size_t Y = 0; Y < 4; Y++) {

      // if the board is empty at this point we have nothing todo
      if (Board[Y][X] == 0)
        continue;

      // get Y offset, we start at line 7 and take every second line
      size_t OffY = Y * 2 + Offset.Y + 4;

      // get X offset, we start at first box each has size 5
      size_t OffX = Offset.X + 1 + X * 5;

      // draw the element
      Scr[OffY][OffX] << getElemColor(Board[Y][X]) << Board[Y][X];
    }
  }

  auto const ScrSize = Scr.getSize();
  Scr[ScrSize.Y - 1][0] << "[Q]uit | Controls: [UP] [DOWN] [LEFT] [RIGHT]";

  // draw message if necessary
  if (State == GameOver) {
    Scr[MsgOffset] << ::cxxg::types::Color::RED << " GAME OVER ";
  } else if (State == NewGame) {
    Scr[MsgOffset] << ::cxxg::types::Color::YELLOW << " NEW  GAME ";

    // we want to be able to keep playing
    State = Running;
  }

  ::cxxg::Game::handleDraw();
}

void Game2048::drawLogoAnimation() {
  // define size of logo and get offset
  ::cxxg::types::Size const LogoSize{38, 7};
  auto const Offset = getOffset(LogoSize);

  // print characters of logo
  Scr[Offset.Y + 0][Offset.X] << " 222222    000000   44    44   888888 ";
  Scr[Offset.Y + 1][Offset.X] << "22    22  000   00  44    44  88    88";
  Scr[Offset.Y + 2][Offset.X] << "      22  0000  00  44    44  88    88";
  Scr[Offset.Y + 3][Offset.X] << " 222222   00 00 00  44444444   888888 ";
  Scr[Offset.Y + 4][Offset.X] << "22        00  0000        44  88    88";
  Scr[Offset.Y + 5][Offset.X] << "22        00   000        44  88    88";
  Scr[Offset.Y + 6][Offset.X] << "22222222   000000         44   888888 ";

  // animate the colors
  for (int L = 0; L < 10; L++) {
    auto Start = Offset;
    auto End = Offset + ::cxxg::types::Size{8, 6};

    // 2
    Scr.setColor(Start, End, getColor(L + 0));

    // 0
    Start += {10, 0};
    End += {10, 0};
    Scr.setColor(Start, End, getColor(L + 1));

    // 4
    Start += {10, 0};
    End += {10, 0};
    Scr.setColor(Start, End, getColor(L + 2));

    // 8
    Start += {10, 0};
    End += {10, 0};
    Scr.setColor(Start, End, getColor(L + 3));

    // update screen and wait some time
    Scr.update();
    ::cxxg::utils::sleep(60000);
  }

  Scr.clear();
}

void Game2048::handleGameOver() {
  State = GameOver;
  HighScore = ::std::max(Score, HighScore);
  GameRunning = false;
}

void Game2048::handleExit() {
  if (!HighScoreFile.empty()) {
    ::std::ofstream File(HighScoreFile.c_str(), ::std::ios::out);
    if (File.is_open()) {
      File << HighScore;
    } else {
      warn() << "Could not open highscore file: '" << HighScoreFile
             << "' for writing.";
    }
  }

  handleDraw();

  ::cxxg::Game::handleExit();
}

void Game2048::addNewElement() {

  // check if we have a free space to add a new element
  // if not the game is over and the player has lost
  if (!hasFreeSpace(Board)) {
    handleGameOver();
    return;
  }

  // get a random empty position on the board
  size_t X, Y;
  do {
    X = RndEngine() & 0x3;
    Y = RndEngine() & 0x3;
  } while (Board[Y][X] != 0);

  // add a random element
  Board[Y][X] = RndEngine() & 0x1 ? 2 : 4;
}

int Game2048::getPowerOfTwo(unsigned Element) {
  int PowerOfTwo = 0;

  // we first shift once since 2 has second bit which gives a power of 1
  for (Element >>= 1; Element; PowerOfTwo++, Element >>= 1)
    ;

  return PowerOfTwo;
}

::cxxg::types::TermColor Game2048::getColor(unsigned Idx) {
  switch (Idx % 4) {
  case 0: // 2, 32, ...
    return ::cxxg::types::Color::BLUE;
  case 1: // 4, 64, ...
    return ::cxxg::types::Color::GREEN;
  case 2: // 8, 128, ...
    return ::cxxg::types::Color::YELLOW;
  case 3: // 16, 256, ...
    return ::cxxg::types::Color::RED;
  default:
    // should not happen
    break;
  }
  return ::cxxg::types::Color::RED;
}

::cxxg::types::TermColor Game2048::getElemColor(unsigned Element) {
  auto PowerOfTwo = getPowerOfTwo(Element);
  return getColor(PowerOfTwo - 1);
}

bool Game2048::hasFreeSpace(::std::vector<::std::vector<unsigned>> &Board) {
  for (auto const &Elems : Board) {
    for (auto Elem : Elems) {
      if (Elem == 0)
        return true;
    }
  }

  return false;
}

unsigned Game2048::moveRight(::std::vector<::std::vector<unsigned>> &Board) {
  unsigned Score = 0;

  for (size_t Y = 0; Y < 4; Y++) {
    Score += moveLineRight(Board.at(Y));
  }

  return Score;
}

unsigned Game2048::moveLeft(::std::vector<::std::vector<unsigned>> &Board) {
  rotateBoard(Board);
  rotateBoard(Board);
  unsigned Score = moveRight(Board);
  rotateBoard(Board);
  rotateBoard(Board);

  return Score;
}

unsigned Game2048::moveUp(::std::vector<::std::vector<unsigned>> &Board) {
  rotateBoard(Board);
  rotateBoard(Board);
  rotateBoard(Board);
  unsigned Score = moveRight(Board);
  rotateBoard(Board);

  return Score;
}

unsigned Game2048::moveDown(::std::vector<::std::vector<unsigned>> &Board) {
  rotateBoard(Board);
  unsigned Score = moveRight(Board);
  rotateBoard(Board);
  rotateBoard(Board);
  rotateBoard(Board);

  return Score;
}

void Game2048::rotateBoard(::std::vector<::std::vector<unsigned>> &Board) {
  auto TmpBoard = Board;

  for (size_t Y = 0; Y < 4; Y++) {
    for (size_t X = 0; X < 4; X++) {
      Board[3 - X][Y] = TmpBoard[Y][X];
    }
  }
}

unsigned Game2048::moveLineRight(::std::vector<unsigned> &Line) {

  // keep track of how much additional score we got
  unsigned Score = 0;

  // start iterating from right to left and try to move elements
  for (int X = Line.size() - 2; X >= 0; X--) {

    // get the current element
    unsigned CurrElem = Line.at(X);

    // check if we need to move sth
    if (CurrElem == 0)
      continue;

    // find next non-empty field
    auto It =
        ::std::find_if(Line.begin() + X + 1, Line.end(),
                       [](unsigned Element) -> bool { return Element != 0; });

    // check if we have found a non-empty field and the element matches ours
    if (It != Line.end() && *It == CurrElem) {

      // yes, merge elements
      *It = CurrElem * 2;
      Line.at(X) = 0;

      // update score
      Score += CurrElem * 2;

      // we are done here
      continue;
    }

    // we havent found a non-empty field or the element doesnt
    // match ours
    Line.at(X) = 0;
    *--It = CurrElem;
  }

  return Score;
}
