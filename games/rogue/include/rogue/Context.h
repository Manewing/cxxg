#ifndef ROGUE_CONTEXT_H
#define ROGUE_CONTEXT_H

namespace rogue {
class EventHub;
class ItemDatabase;
class EntityDatabase;
class LevelDatabase;
class CraftingDatabase;
class CraftingHandler;
} // namespace rogue

namespace rogue {

struct GameContext {
  EventHub &EvHub;
  ItemDatabase &ItemDb;
  EntityDatabase &EntityDb;
  LevelDatabase &LevelDb;
  CraftingDatabase &CraftingDb;
  CraftingHandler &Crafter;
};

} // namespace rogue

#endif // #findef ROGUE_CONTEXT_H