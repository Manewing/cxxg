#ifndef CXXG_GAME_H
#define CXXG_GAME_H

#include <cxxg/Screen.h>

#include <memory>
#include <random>
#include <string>
#include <vector>

namespace cxxg {

/// Base class for console games, provides game loop, screen to draw, a random
/// number engine and messaging to user.
class Game {
public:
  /// Creates new game instance, uses current terminal size for screen size,
  /// and initilizes random engine with current time stamp. The Flag
  // 'GameRunning' is set to true.
  Game();

  /// Initializes environment and configures the input type to the requested
  /// one.
  /// @param[in] BufferedInput - If to use buffered input, i.e. a '\n' needs
  ///   to follow after input, otherwise uses un-buffered.
  virtual void initialize(bool BufferedInput = false);

  /// Game loop, while flag 'GameRunning' is true calls 'handleInput'
  /// and 'handleDraw' continously.
  void run();

  /// Callback for handling new character input, called from game loop.
  /// @param[in] Char - The new character input
  virtual void handleInput(int Char) = 0;

  /// Callback for handling drawing to the game screen, called from game loop.
  virtual void handleDraw();

  /// Handles game exit, restores environment to its original state.
  virtual void handleExit();

  /// Creates a warning and returns a writable accessor to it.
  RowAccessor warn();

  /// Checks if the screen size is sufficient for the required game size.
  /// @param[in] GameSize - The required game size to check the screen size for
  /// @throws ::std::out_of_range if screen size is insufficient
  /// TODO rename to assertSize and make add function checkSize return bool
  void checkSize(ScreenSize GameSize) const;

  /// Calculates the offset into the game screen for the required game size
  /// such that the game is positioned central.
  /// @param[in] GameSize - The game size to get the offset for
  /// @throws ::std::out_of_range if screen size is insufficient for game size
  ScreenSize getOffset(ScreenSize GameSize) const;

  /// Sets the seed for the random number generator
  /// @param[in] Seed - The seed to set
  void setRandomSeed(size_t Seed);

protected:
  /// Screen on which the game will be displayed
  Screen Scr;

  /// Flag for game loop determining whether the game is running
  bool GameRunning;

  /// Random engine for generating random numbers in games
  ::std::default_random_engine RndEngine;

private:
  /// Buffer for warnings to display
  ::std::vector<Row> Warnings;
};

}; // namespace cxxg

#endif // #ifndef CXXG_GAME_H
