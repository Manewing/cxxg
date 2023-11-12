#include <rogue/GameWorld.h>
#include <rogue/Level.h>
#include <rogue/LevelGenerator.h>

namespace rogue {

std::unique_ptr<GameWorld> GameWorld::create(LevelGenerator &LvlGen,
                                             std::string_view Type) {
  if (Type == MultiLevelDungeon::Type) {
    return std::make_unique<MultiLevelDungeon>(LvlGen);
  }

  if (Type == DungeonSweeper::Type) {
    return std::make_unique<DungeonSweeper>(LvlGen);
  }

  throw std::runtime_error("GameWorld: Unknown type: " + std::string(Type));
}

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

void MultiLevelDungeon::switchWorld(unsigned, std::string_view Type,
                                    std::filesystem::path Config) {
  throw std::runtime_error("MultiLevelDungeon: switchWorld not implemented: " +
                           std::string(Type) + " -> " + Config.string());
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

DungeonSweeper::DungeonSweeper(LevelGenerator &LevelGen) : LevelGen(LevelGen) {}

Level &DungeonSweeper::switchLevel(std::size_t LevelIdx) {
  if (CurrSubWorld) {
    return CurrSubWorld->switchLevel(LevelIdx);
  }

  if (LevelIdx != 0) {
    throw std::runtime_error("DungeonSweeper: LevelIdx out of range");
  }

  if (!Lvl) {
    Lvl = LevelGen.generateLevel(LevelIdx);
    Lvl->setEventHub(Hub);
  }
  // FIXME
  CurrentLevelIdx = LevelIdx;

  return *Lvl;
}

void DungeonSweeper::switchWorld(unsigned Seed, std::string_view Type,
                                 std::filesystem::path Config) {
  if (Type == DungeonSweeper::Type) {
    CurrSubLvlGen = nullptr;
    CurrSubWorld = nullptr;
  } else {
    CurrSubLvlGen = LevelGeneratorLoader(LevelGen.getCtx()).load(Seed, Config);
    CurrSubWorld = GameWorld::create(*CurrSubLvlGen, Type);
    CurrSubWorld->setEventHub(Hub);
  }
}

std::size_t DungeonSweeper::getCurrentLevelIdx() const {
  if (CurrSubWorld) {
    return CurrSubWorld->getCurrentLevelIdx();
  }
  return CurrentLevelIdx;
}

Level *DungeonSweeper::getCurrentLevel() {
  return const_cast<Level *>(
      static_cast<const DungeonSweeper *>(this)->getCurrentLevel());
}

const Level *DungeonSweeper::getCurrentLevel() const {
  if (CurrSubWorld) {
    return CurrSubWorld->getCurrentLevel();
  }
  return Lvl.get();
}

Level &DungeonSweeper::getCurrentLevelOrFail() {
  return const_cast<Level &>(
      static_cast<const DungeonSweeper *>(this)->getCurrentLevelOrFail());
}

const Level &DungeonSweeper::getCurrentLevelOrFail() const {
  const auto *Lvl = getCurrentLevel();
  if (!Lvl) {
    throw std::runtime_error("DungeonSweeper: No current level selected");
  }
  return *Lvl;
}

} // namespace rogue