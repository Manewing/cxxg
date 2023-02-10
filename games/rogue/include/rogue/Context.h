#ifndef ROGUE_CONTEXT_H
#define ROGUE_CONTEXT_H

namespace rogue {
class Game;
class ItemDatabase;
} // namespace rogue

namespace rogue {

struct GameContext {
  Game &G;
  ItemDatabase &ItemDb;
};

} // namespace rogue

#endif // #findef ROGUE_CONTEXT_H