#ifndef ROGUE_CONTEXT_H
#define ROGUE_CONTEXT_H

namespace rogue {
class Game;
class ItemDatabase;
class CreatureDatabase;
} // namespace rogue

namespace rogue {

struct GameContext {
  Game &G;
  const ItemDatabase &ItemDb;
  const CreatureDatabase &CreatureDb;
};

} // namespace rogue

#endif // #findef ROGUE_CONTEXT_H