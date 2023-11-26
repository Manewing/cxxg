#ifndef ROGUE_CONTEXT_H
#define ROGUE_CONTEXT_H

namespace rogue {
class ItemDatabase;
class EntityDatabase;
class CreatureDatabase;
class LevelDatabase;
} // namespace rogue

namespace rogue {

struct GameContext {
  ItemDatabase &ItemDb;
  EntityDatabase &EntityDb;
  CreatureDatabase &CreatureDb;
  LevelDatabase &LevelDb;
};

} // namespace rogue

#endif // #findef ROGUE_CONTEXT_H