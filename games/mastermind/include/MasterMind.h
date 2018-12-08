#ifndef MASTERMIND_H
#define MASTERMIND_H

#include <cxxg/Game.h>
#include <map>

class MasterMind : public ::cxxg::Game {
public:
  typedef char Pin;
  typedef ::std::vector<Pin> Pins;

  typedef enum { Running, GameOver, Victory } GameState;

  struct HistGuess {
    Pins Guess;
    size_t Blacks;
    size_t Whites;
    size_t TimeStamp;
  };

  static ::std::map<char, ::cxxg::types::Color> Colors;

  static ::std::string formatTime(size_t Seconds);

public:
  MasterMind();
  virtual ~MasterMind();

  void initialize(bool BufferedInput = false) final;
  void handleInput(int Char) final;
  void handleReturn();
  void handleLeft();
  void handleRight();

  void handleDraw() final;
  void drawLogoAnimation();

  void handleGameOver();

  static bool hasDuplicates(Pins const &Guess);
  static bool isComplete(Pins const &Guess);
  static bool isValidGuess(Pins const &Guess);

  static size_t getBlacks(Pins const &Code, Pins const &Guess);
  static size_t getWhites(Pins const &Code, Pins const &Guess);

  Pins getRandomCode(size_t CodeSize, size_t PinCount);

private:
  GameState State;
  int InputPosition;
  Pins Code;
  Pins CurrentGuess;
  size_t GameStartTimeStamp;
  ::std::vector<HistGuess> History;
};

#endif // #ifndef MASTERMIND_H
