#ifndef ROGUE_CONTEXT_H
#define ROGUE_CONTEXT_H

namespace rogue {
class ItemDatabase;
class CreatureDatabase;
class LevelDatabase;
} // namespace rogue

namespace rogue {

struct GameContext {
  const ItemDatabase &ItemDb;
  const CreatureDatabase &CreatureDb;
  const LevelDatabase &LevelDb;
};

} // namespace rogue

#endif // #findef ROGUE_CONTEXT_H