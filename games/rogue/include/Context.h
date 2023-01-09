#ifndef ROGUE_CONTEXT_H
#define ROGUE_CONTEXT_H

class Game;
class ItemDatabase;

struct GameContext {
  Game &G;
  ItemDatabase &ItemDb;
};

#endif // #findef ROGUE_CONTEXT_H