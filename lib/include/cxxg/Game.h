#include <cxxg/Screen.h>

namespace terminos {
struct terminos;
}

namespace cxxg {

class Game {
public:
  Game(size_t SizeX, size_t SizeY);
  virtual ~Game();

  /// Initializes environment
  virtual void initialize(bool BufferedInput = true);

  void run();

  virtual bool handleInput(int Char) = 0;

  virtual void draw() = 0;

  virtual void handleGameOver(bool Victory = false);

protected:
  void switchBufferedInput();

protected:
  Screen Scr;
  ::std::shared_ptr<::terminos::terminos> TermAttrOld;
  bool GameRunning;
};

}; // namespace cxxg
