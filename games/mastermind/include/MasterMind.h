#ifndef MASTERMIND_H
#define MASTERMIND_H

#include <cxxg/Game.h>
#include <map>
#include <ctime>

class MasterMind : public ::cxxg::Game {
public:
  typedef char Pin;
  typedef ::std::vector<Pin> Pins;

  typedef enum { Running, GameOver, Victory } GameState;

  struct HistGuess {
    Pins Guess;
    size_t Blacks;
    size_t Whites;
    std::time_t TimeStamp;
  };

  static ::std::map<char, ::cxxg::types::TermColor> Colors;

  static ::std::string formatMinutesSeconds(std::time_t TimeStamp);

public:
  MasterMind(cxxg::Screen &Scr);
  virtual ~MasterMind();

  void initialize(bool BufferedInput = false, unsigned TickDelayUs = 0) final;
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
  std::time_t GameStartTimeStamp;
  ::std::vector<HistGuess> History;
};

#endif // #ifndef MASTERMIND_H
