#include <rogue/GameWorld.h>
#include <rogue/Level.h>
#include <rogue/LevelGenerator.h>
#include <rogue/Components/Transform.h>

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

Level &MultiLevelDungeon::switchLevel(std::size_t LevelIdx, bool ToEntry) {
  if (LevelIdx >= Levels.size() + 1) {
    throw std::runtime_error("MultiLevelDungeon: LevelId out of range");
  }

  auto *CurrLvl = getCurrentLevel();
  if (LevelIdx >= Levels.size()) {
    Levels.push_back(LevelGen.generateLevel(LevelIdx));
    Levels.back()->setEventHub(Hub);
  }
  auto &Nextlvl = *Levels.at(LevelIdx);

  if (CurrLvl && CurrLvl != &Nextlvl && CurrLvl->getPlayer() != entt::null) {
    auto ToPos =
        ToEntry ? Nextlvl.getPlayerStartPos() : Nextlvl.getPlayerEndPos();
    Nextlvl.update(false);
    Nextlvl.movePlayer(*CurrLvl, ToPos);
  }

  CurrentLevelIdx = LevelIdx;
  return getCurrentLevelOrFail();
}

void MultiLevelDungeon::switchWorld(unsigned, std::string_view Type,
                                    std::filesystem::path Config,
                                    entt::entity) {
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

Level &DungeonSweeper::switchLevel(std::size_t LevelIdx, bool ToEntry) {
  if (CurrSubWorld && LevelIdx < CurrMaxLevel) {
    return CurrSubWorld->switchLevel(LevelIdx, ToEntry);
  } else if (CurrSubWorld) {
    // Reached max level of dungeon that means exit level
    auto *CurrLvl = getCurrentLevel();
    auto ToPos = Lvl->Reg.get<PositionComp>(CurrSwitchEntity).Pos;
    Lvl->update(false);
    Lvl->movePlayer(*CurrLvl, ToPos);
    Lvl->Reg.destroy(CurrSwitchEntity);
    CurrSubWorld = nullptr;
    CurrSubLvlGen = nullptr;
    LevelIdx = 0;
  }

  if (LevelIdx != 0) {
    throw std::runtime_error("DungeonSweeper: LevelIdx out of range");
  }

  if (!Lvl) {
    Lvl = LevelGen.generateLevel(LevelIdx);
    Lvl->setEventHub(Hub);
  }

  // We did not switch levels so no need to move player
  return *Lvl;
}

void DungeonSweeper::switchWorld(unsigned Seed, std::string_view Type,
                                 std::filesystem::path Config,
                                 entt::entity SwitchEt) {
  if (Type == DungeonSweeper::Type) {
    // FIXME
    CurrSubLvlGen = nullptr;
    CurrSubWorld = nullptr;
    return;
  }

  // If we have a sub-world active move player to local level
  if (CurrSubWorld) {
    auto *CurrLvl = getCurrentLevel();
    // FIXME move to position of original entry
    auto ToPos = Lvl->getPlayerStartPos();
    Lvl->update(false);
    Lvl->movePlayer(*CurrLvl, ToPos);
  }

  auto SwitchPos = Lvl->Reg.get<PositionComp>(SwitchEt).Pos;
  Seed ^= Seed << 12 ^ SwitchPos.X ^ SwitchPos.Y << 8;

  CurrSubLvlGen = LevelGeneratorLoader(LevelGen.getCtx()).load(Seed, Config);
  CurrSubWorld = GameWorld::create(*CurrSubLvlGen, Type);
  CurrSubWorld->setEventHub(Hub);
  CurrMaxLevel = 1;
  if (auto *CMLG =
          dynamic_cast<CompositeMultiLevelGenerator *>(CurrSubLvlGen.get())) {
    CurrMaxLevel = CMLG->getMaxLevelIdx();
  }
  CurrSwitchEntity = SwitchEt;

  // Move player to the new sub-world
  auto &NextLvl = CurrSubWorld->switchLevel(0, true);
  auto ToPos = NextLvl.getPlayerStartPos();
  NextLvl.update(false);
  NextLvl.movePlayer(*Lvl, ToPos);
}

std::size_t DungeonSweeper::getCurrentLevelIdx() const {
  if (CurrSubWorld) {
    return CurrSubWorld->getCurrentLevelIdx();
  }
  return 0;
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