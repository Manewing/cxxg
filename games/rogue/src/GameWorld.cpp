#include <rogue/GameWorld.h>
#include <rogue/Level.h>
#include <rogue/LevelGenerator.h>

namespace rogue {

MultiLevelDungeon::MultiLevelDungeon(LevelGenerator &LvlGen)
    : LevelGen(LvlGen) {}

Level &MultiLevelDungeon::switchLevel(std::size_t LevelIdx) {
  if (LevelIdx >= Levels.size() + 1) {
    throw std::runtime_error("MultiLevelDungeon: LevelId out of range");
  }
  if (LevelIdx >= Levels.size()) {
    Levels.push_back(LevelGen.generateLevel(LevelIdx));
    Levels.back()->setEventHub(Hub);
  }

  CurrentLevelIdx = LevelIdx;
  return getCurrentLevelOrFail();
}

std::size_t MultiLevelDungeon::getCurrentLevelIdx() const {
  return CurrentLevelIdx;
}

Level *MultiLevelDungeon::getCurrentLevel() {
  return const_cast<Level *>(
      static_cast<const MultiLevelDungeon *>(this)->getCurrentLevel());
}

const Level *MultiLevelDungeon::getCurrentLevel() const {
  if (Levels.empty() || CurrentLevelIdx >= Levels.size()) {
    return nullptr;
  }

  return Levels.at(CurrentLevelIdx).get();
}

Level &MultiLevelDungeon::getCurrentLevelOrFail() {
  return const_cast<Level &>(
      static_cast<const MultiLevelDungeon *>(this)->getCurrentLevelOrFail());
}

const Level &MultiLevelDungeon::getCurrentLevelOrFail() const {
  const auto *Lvl = getCurrentLevel();
  if (!Lvl) {
    throw std::runtime_error("MultiLevelDungeon: No current level selected");
  }
  return *Lvl;
}

} // namespace rogue