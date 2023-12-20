#ifndef ROGUE_CONTEXT_H
#define ROGUE_CONTEXT_H

namespace rogue {
class ItemDatabase;
class EntityDatabase;
class LevelDatabase;
class CraftingHandler;
} // namespace rogue

namespace rogue {

struct GameContext {
  ItemDatabase &ItemDb;
  EntityDatabase &EntityDb;
  LevelDatabase &LevelDb;
  CraftingHandler &Crafter;
};

} // namespace rogue

#endif // #findef ROGUE_CONTEXT_H