#ifndef ROGUE_CONTEXT_H
#define ROGUE_CONTEXT_H

namespace rogue {
class ItemDatabase;
class CreatureDatabase;
class LevelDatabase;
} // namespace rogue

namespace rogue {

struct GameContext {
  ItemDatabase &ItemDb;
  CreatureDatabase &CreatureDb;
  LevelDatabase &LevelDb;
};

} // namespace rogue

#endif // #findef ROGUE_CONTEXT_H