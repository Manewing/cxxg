#include <Tetris.h>
#include <array>
#include <cxxg/Utils.h>

Tetris::Tetris(cxxg::Screen &Scr)
    : cxxg::Game(Scr), F(14, 20), T(randomTetromino()) {
  T.setPosition({5, 0});
}

Tetris::~Tetris() {}

void Tetris::handleInput(int Char) {
  switch (Char) {
  case cxxg::utils::KEY_LEFT:
    tryMove({-1, 0});
    break;
  case cxxg::utils::KEY_RIGHT:
    tryMove({1, 0});
    break;
  case cxxg::utils::KEY_DOWN:
    tryMove({0, 1});
    break;
  case cxxg::utils::KEY_UP:
    tryRotate(1);
    break;
  case cxxg::utils::KEY_SPACE:
    while (tryMove({0, 1}))
      ;
    if (moveTetrominoDown()) {
      handleFullLines();
      placeNewTetromino();
    }
    return;
  default:
    break;
  }

  if (++TickCounter % InvSpeed == 0) {
    if (moveTetrominoDown()) {
      handleFullLines();
      placeNewTetromino();
    }
  }
}

void Tetris::handleDraw() {
  const cxxg::types::Size FieldSize{16, 22};
  const cxxg::types::Size FieldOffset = getOffset(FieldSize);

  F.draw(Scr, FieldOffset);
  T.draw(Scr, FieldOffset);

  const cxxg::types::Size GameSize = FieldSize + cxxg::types::Size{40, 10};
  const cxxg::types::Size GameOffset = getOffset(GameSize);
  drawLogo(GameOffset);

  Scr[GameOffset.Y + 7][GameOffset.X] << "[SCORE]: " << Score;
  Scr[GameOffset.Y + 8][GameOffset.X] << "[COUNT]: " << Count;
  Scr[GameOffset.Y + 9][GameOffset.X] << "[SPEED]: " << InvSpeed;

  if (!GameRunning) {
    Scr[GameOffset.Y + 10][GameOffset.X + 21] << "[GAME OVER]";
  }

  cxxg::Game::handleDraw();
}

void Tetris::drawLogo(cxxg::types::Position Offset) {
  Scr[Offset.Y + 0][Offset.X + 10] << " _____ _____ _____ ____  ___ ____  ";
  Scr[Offset.Y + 1][Offset.X + 10] << "|_   _| ____|_   _|  _ \\|_ _/ ___| ";
  Scr[Offset.Y + 2][Offset.X + 10] << "  | | |  _|   | | | |_) || |\\___ \\ ";
  Scr[Offset.Y + 3][Offset.X + 10] << "  | | | |___  | | |  _ < | | ___) |";
  Scr[Offset.Y + 4][Offset.X + 10] << "  |_| |_____| |_| |_| \\_|___|____/ ";
}

bool Tetris::tryMove(cxxg::types::Position Delta) {
  T += Delta;
  if (!F.inBounds(T) || F.collides(T)) {
    T -= Delta;
    return false;
  }
  return true;
}

bool Tetris::tryRotate(int Times) {
  T.rotate(Times);
  if (!F.inBounds(T) || F.collides(T)) {
    T.rotate(-Times);
    return false;
  }
  return true;
}

bool Tetris::moveTetrominoDown() {
  T += {0, 1};
  if (F.inBounds(T) && !F.collides(T)) {
    return false;
  }
  T -= {0, 1};
  F.place(T);
  Score += 25;
  return true;
}

void Tetris::placeNewTetromino() {
  T = randomTetromino();
  T.setPosition({5, 0});

  if (F.collides(T)) {
    GameRunning = false;
  }

  Count++;
  if (Count % 10 == 0) {
    if (InvSpeed > 10) {
      InvSpeed--;
    }
  }
}

Tetromino Tetris::randomTetromino() {
  using UniUint = std::uniform_int_distribution<unsigned>;
  UniUint ColorUni(0, Tetromino::Colors.size() - 1);
  unsigned ColorIdx = ColorUni(RndEngine);
  UniUint ShapeUni(0, Tetromino::NumShapes - 1);
  unsigned ShapeIdx = ShapeUni(RndEngine);
  return Tetromino::create(Tetromino::Colors.at(ColorIdx), ShapeIdx);
}

void Tetris::handleFullLines() {
  auto FullLines = F.getFullLines();
  if (FullLines.empty()) {
    return;
  }

  // Update score
  Score += (1 << FullLines.size()) * 100;

  for (unsigned Tick = 0; Tick < 20; Tick++) {
    for (unsigned Y : FullLines) {
      F.animateRainbowLine(Y, Tick);
    }
    cxxg::utils::sleep(30000);
    handleDraw();
  }

  F.removeFullLines();
}