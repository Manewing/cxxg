#ifndef TETRIS_H
#define TETRIS_H

#include <cxxg/Game.h>
#include <cxxg/Types.h>
#include <random>

#include <Field.h>
#include <Tetromino.h>

class Tetris : public ::cxxg::Game {
public:
  Tetris(cxxg::Screen &Scr);
  virtual ~Tetris();

  bool handleInput(int Char) final;

  void handleDraw() final;
  void drawLogo(cxxg::types::Position Offset);

  bool tryMove(cxxg::types::Position Delta);
  bool tryRotate(int Times);

  bool moveTetrominoDown();
  void placeNewTetromino();
  Tetromino randomTetromino();

  void handleFullLines();

private:
  Field F;
  Tetromino T;

  unsigned Score = 0;
  unsigned TickCounter = 0;
  unsigned InvSpeed = 20;
  unsigned Count = 0;
};

#endif // #ifndef TETRIS_H