#ifndef ROGUE_LEVEL_GENERATOR_H
#define ROGUE_LEVEL_GENERATOR_H

#include <filesystem>
#include <map>
#include <memory>
#include <rogue/Level.h>
#include <rogue/Tile.h>
#include <vector>
#include <ymir/Types.hpp>

namespace rogue {
struct GameContext;
} // namespace rogue

namespace rogue {

struct LevelEntityConfig {
  std::map<char, std::string> Entities;
};

/// Base class for all level generators
class LevelGenerator {
public:
  explicit LevelGenerator(const GameContext &Ctx);

  const GameContext &getCtx() const { return Ctx; }

  virtual ~LevelGenerator() = default;
  virtual std::shared_ptr<Level> generateLevel(int LevelId) const = 0;

protected:
  void spawnEntities(const LevelEntityConfig &Cfg, Level &L) const;
  void spawnEntity(char Char, const LevelEntityConfig &Cfg, Level &L,
                   ymir::Point2d<int> Pos) const;

protected:
  const GameContext &Ctx;
};

/// Level generator that generates an empty level with the given size
class EmptyLevelGenerator : public LevelGenerator {
public:
  struct Config {
    ymir::Size2d<int> Size;
  };

public:
  EmptyLevelGenerator(const GameContext &Ctx, const Config &Cfg);
  std::shared_ptr<Level> generateLevel(int LevelId) const final;

private:
  Config Cfg;
};

/// Level generator that generates a level from a designed map
class DesignedMapLevelGenerator : public LevelGenerator {
public:
  struct Config {
    struct CharInfo {
      Tile T;
      std::string Layer;
    };

    std::filesystem::path MapFile;
    CharInfo DefaultChar;
    std::map<char, CharInfo> CharInfoMap;
    LevelEntityConfig EntityConfig;
  };

public:
  DesignedMapLevelGenerator(const GameContext &Ctx, const Config &Cfg);

  std::shared_ptr<Level> generateLevel(int LevelId) const final;

protected:
  std::shared_ptr<Level> createNewLevel(int LevelId) const;

private:
  Config Cfg;
};

/// Level generator that generates a level from a procedurally generated map
class GeneratedMapLevelGenerator : public LevelGenerator {
public:
  static bool DebugRooms;

public:
  struct Config {
    unsigned Seed = 0;
    std::filesystem::path MapConfig;
    LevelEntityConfig EntityConfig;
  };

public:
  GeneratedMapLevelGenerator(const GameContext &Ctx, const Config &Cfg);

  std::shared_ptr<Level> generateLevel(int LevelId) const final;

protected:
  std::shared_ptr<Level> createNewLevel(int LevelId) const;

private:
  Config Cfg;
};

/// A level generator that is build up from multiple level generators and can
/// use different generators for different level ranges
class CompositeMultiLevelGenerator : public LevelGenerator {
public:
  struct Config {
    struct LevelRange {
      std::size_t LevelEndIdx = 0;
      std::filesystem::path Config;
    };
    std::vector<LevelRange> Levels;
  };

  struct LevelRange {
    std::size_t LevelEndIdx = 0;
    std::shared_ptr<LevelGenerator> Generator;
  };

public:
  CompositeMultiLevelGenerator(const GameContext &Ctx);

  const LevelGenerator &getGeneratorForLevel(std::size_t LevelIdx) const;

  void addGenerator(std::shared_ptr<LevelGenerator> Generator,
                    std::size_t LevelEndIdx);

  std::shared_ptr<Level> generateLevel(int LevelId) const final;

  std::size_t getMaxLevelIdx() const;

private:
  std::vector<LevelRange> Generators;
};

/// Helper class for loading level generation configurations and creating
/// level generators
class LevelGeneratorLoader {
public:
  using LevelConfig = std::variant<
      EmptyLevelGenerator::Config, DesignedMapLevelGenerator::Config,
      GeneratedMapLevelGenerator::Config, CompositeMultiLevelGenerator::Config>;

public:
  static LevelConfig loadCfg(unsigned Seed,
                             const std::filesystem::path &CfgFile);

public:
  explicit LevelGeneratorLoader(const GameContext &Ctx);

  std::shared_ptr<LevelGenerator> create(unsigned Seed, const LevelConfig &Cfg);
  std::shared_ptr<LevelGenerator> load(unsigned Seed,
                                       const std::filesystem::path &CfgFile);

private:
  const GameContext &Ctx;
};

} // namespace rogue

#endif // #ifndef ROGUE_LEVEL_GENERATOR_H