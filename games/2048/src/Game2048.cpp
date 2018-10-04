#include "Game2048.h"

#include <algorithm>
#include <fstream>

// TODO get terminal size and adapt to it
Game2048::Game2048(::std::string const &HighScoreFile)
    : Game(::cxxg::ScreenSize{80, 24}), Score(0), HighScore(0), State(Running),
      HighScoreFile(HighScoreFile) {
  Board.resize(4, ::std::vector<unsigned>{0, 0, 0, 0});

  if (!HighScoreFile.empty()) {
    ::std::ifstream File(HighScoreFile.c_str(), ::std::ios::in);
    if (File.is_open()) {
      File >> HighScore;
    } else {
      ::std::stringstream SS;
      SS << "Could not open highscore file: '" << HighScoreFile
         << "' for reading.";
      warn(SS.str());
    }
  }
}

Game2048::~Game2048() { handleExit(); }

void Game2048::initialize(bool BufferedInput) {
  ::cxxg::Game::initialize(BufferedInput);

  // add first two initial elements to the board
  addNewElement();
  addNewElement();

  draw();
}

void Game2048::handleInput(int Char) {
  if (Char == -1) {
    handleGameOver(false);
  }

  bool HasMoved = true;
  switch (Char) {
  case 68:
    Score += moveLeft(Board);
    break;
  case 67:
    Score += moveRight(Board);
    break;
  case 65:
    Score += moveUp(Board);
    break;
  case 66:
    Score += moveDown(Board);
    break;
  case 'q':
    handleGameOver(false);
  default:
    HasMoved = false;
    break;
  }

  if (HasMoved) {
    addNewElement();
  }
}

void Game2048::draw() {
  auto const Offset = 29, MsgOffsetX = 34, MsgOffsetY = 10;

  // draw the score board
  Scr[3][Offset] << ::cxxg::Color::BLUE << "Score: " << Score;
  Scr[4][Offset] << ::cxxg::Color::GREEN << "HighScore: " << HighScore;

  // draw the board outline
  for (size_t Y = 0; Y < 8; Y += 2) {
    Scr[6 + Y][Offset] << "+----+----+----+----+";
    Scr[7 + Y][Offset] << "|    |    |    |    |";
  }
  Scr[14][Offset] << "+----+----+----+----+";

  // fill the board
  for (size_t X = 0; X < 4; X++) {
    for (size_t Y = 0; Y < 4; Y++) {

      // if the board is empty at this point we have nothing todo
      if (Board[Y][X] == 0)
        continue;

      // get Y offset, we start at line 7 and take every second line
      size_t OffY = Y * 2 + 7;

      // get X offset, we start at first box each has size 5
      size_t OffX = Offset + 1 + X * 5;

      // draw the element
      Scr[OffY][OffX] << getColor(Board[Y][X]) << Board[Y][X];
    }
  }

  // draw message if necessary
  if (State == GameOver) {
    Scr[MsgOffsetY][MsgOffsetX] << ::cxxg::Color::RED << " GAME OVER ";
  } else if (State == Victory) {
    Scr[MsgOffsetY][MsgOffsetX + 1] << ::cxxg::Color::RED << " VICTORY ";

    // we want to be able to keep playing
    State = Running;
  }

  ::cxxg::Game::draw();
}

void Game2048::handleGameOver(bool VC) {
  if (VC) {
    State = Victory;
  } else {
    State = GameOver;
    HighScore = ::std::max(Score, HighScore);
    GameRunning = false;
  }
}

void Game2048::handleExit() {
  if (!HighScoreFile.empty()) {
    ::std::ofstream File(HighScoreFile.c_str(), ::std::ios::out);
    if (File.is_open()) {
      File << HighScore;
    } else {
      ::std::stringstream SS;
      SS << "Could not open highscore file: '" << HighScoreFile
         << "' for writing.";
      warn(SS.str());
    }
  }

  draw();
}

void Game2048::addNewElement() {

  // check if we have a free space to add a new element
  // if not the game is over and the player has lost
  if (!hasFreeSpace(Board)) {
    handleGameOver(false);
  }

  // get a random empty position on the board
  size_t X, Y;
  do {
    X = rand() & 0x3;
    Y = rand() & 0x3;
  } while (Board[Y][X] != 0);

  // add a random element
  Board[Y][X] = rand() & 0x1 ? 2 : 4;
}

int Game2048::getPowerOfTwo(unsigned Element) {
  int PowerOfTwo = 0;

  // we first shift once since 2 has second bit which gives a power of 1
  for (Element >>= 1; Element; PowerOfTwo++, Element >>= 1)
    ;

  return PowerOfTwo;
}

::cxxg::Color Game2048::getColor(unsigned Element) {
  auto PowerOfTwo = getPowerOfTwo(Element);

  // get color for the power of two
  switch ((PowerOfTwo - 1) % 4) {
  case 0: // 2, 32, ...
    return ::cxxg::Color::BLUE;
  case 1: // 4, 64, ...
    return ::cxxg::Color::GREEN;
  case 2: // 8, 128, ...
    return ::cxxg::Color::YELLOW;
  case 3: // 16, 256, ...
    return ::cxxg::Color::RED;
  default:
    // should not happen
    break;
  }

  return ::cxxg::Color::RED;
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
