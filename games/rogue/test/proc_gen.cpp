#include <rogue/LevelGenerator.h>
#include <rogue/NPCEntity.h>
#include <rogue/Renderer.h>
#include <cxxg/Game.h>
#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <memory>
#include <ymir/LayeredMap.hpp>
#include <ymir/Map.hpp>

#include "perlin_noise.h"

using namespace rogue;

template <typename T, typename U>
cxxg::Screen &operator<<(cxxg::Screen &Scr, const ymir::Map<T, U> &Map) {
  for (auto PY = 0; PY < Map.getSize().H; PY++) {
    for (auto PX = 0; PX < Map.getSize().W; PX++) {
      Scr[PY][PX] = Map.getTile({PX, PY});
    }
  }
  return Scr;
}

const Tile DirtTile{{' ', cxxg::types::RgbColor{0, 0, 0, true, 100, 80, 50}}};
const Tile SandTile{{' ', cxxg::types::RgbColor{0, 0, 0, true, 180, 180, 90}}};
const Tile GreenGrassTile{
    {' ', cxxg::types::RgbColor{0, 0, 0, true, 110, 160, 60}}};
const Tile BrownGrassTile{
    {' ', cxxg::types::RgbColor{0, 0, 0, true, 76, 110, 30}}};
const Tile RockTile{{' ', cxxg::types::RgbColor{0, 0, 0, true, 140, 140, 140}}};

// const Tile WallTile{{'#', cxxg::types::RgbColor{60, 60, 60, true, 45, 45,
// 45}}};
const Tile WaterTile{
    {'~', cxxg::types::RgbColor{40, 132, 191, true, 30, 110, 150}}};
// const Tile TreeTile{
//     {'A', cxxg::types::RgbColor{10, 50, 15, true, 100, 80, 50}}};
// const Tile BerryBushTile{
//     {'#', cxxg::types::RgbColor{140, 10, 50, true, 10, 50, 15}}};
// const Tile NPCTile{{'@', cxxg::types::RgbColor{0, 60, 255, true, 100, 80,
// 50}}};

const Tile ChunkMarker{{'x', cxxg::types::RgbColor{255, 0, 0, true, 0, 0, 0}}};

const std::vector<std::string> Layers = {
    "ground", "ground_deco", "walls", "objects", "walls_deco", "enemies",
};

ymir::Map<float> generateSimplexNoiseMap(ymir::Size2d<int> Size,
                                         ymir::Point2d<int> Offset = {0, 0},
                                         int Seed = 0, float Scale = 1.0f,
                                         int Octaves = 1,
                                         float Persistence = 0.5f,
                                         float Lacunarity = 2.0f) {
  ymir::Map<float> M(Size);
  M.forEach([Seed, Scale, Offset, Octaves, Persistence, Lacunarity](auto Pos,
                                                                    auto &T) {
    auto GblPos = Pos + Offset;
    T = noise3(float(GblPos.X) / Scale, float(GblPos.Y) / Scale, Seed, Octaves,
               Persistence, Lacunarity);
  });
  return M;
}

void fillGround(Level &L, const ymir::Map<float> &HeightMap) {
  L.Map.get("ground").forEach([HeightMap](auto Pos, auto &T) {
    auto Height = HeightMap.getTile(Pos);
    if (Height < 0.05) {
      T = SandTile;
    } else if (Height < 0.3) {
      T = GreenGrassTile;
    } else if (Height < 0.5) {
      T = BrownGrassTile;
    } else if (Height < 0.65) {
      T = DirtTile;
    } else {
      T = RockTile;
    }
  });
}

void fillWater(Level &L, const ymir::Map<float> &HeightMap) {
  L.Map.get("objects").forEach([HeightMap](auto Pos, auto &T) {
    if (HeightMap.getTile(Pos) < 0) {
      T = WaterTile;
    }
  });
}

struct Chunk {
  ymir::Map<float> HeightMap;
  std::shared_ptr<Level> L;
  // TODO biome
};

std::shared_ptr<Level> generateChunk(int LevelId, ymir::Rect2d<int> ChunkRect) {
  const auto Size = ChunkRect.Size;
  auto NewChunk = std::make_shared<Level>(LevelId, Layers, Size);

  //  auto HeightMap = generateSimplexNoiseMap(Size, ChunkRect.Pos, 0, 2048.0f,
  //  6);
  auto HeightMap = generateSimplexNoiseMap(Size, ChunkRect.Pos, 0, 128.0f, 6);

  fillGround(*NewChunk, HeightMap);
  fillWater(*NewChunk, HeightMap);

  // DEBUG chunk boarder marker
  NewChunk->Map.get("walls_deco").setTile({0, 0}, ChunkMarker);
  NewChunk->Map.get("walls_deco").setTile({0, Size.H - 1}, ChunkMarker);
  NewChunk->Map.get("walls_deco").setTile({Size.W - 1, 0}, ChunkMarker);
  NewChunk->Map.get("walls_deco")
      .setTile({Size.W - 1, Size.H - 1}, ChunkMarker);

  return NewChunk;
}

class GameWorld {
public:
  virtual ~GameWorld() = default;
  virtual void initialize(ymir::Point2d<int> InitPos = {0, 0}) = 0;
  virtual bool update(ymir::Point2d<int> Pos = {0, 0}) = 0;

  virtual ymir::Map<Tile> renderMap(ymir::Rect2d<int> VisibleRect) const = 0;

  ymir::Map<Tile> setupRenderMap(ymir::Size2d<int> MapSize) const {
    ymir::Map<Tile> RenderedMap(MapSize);
    static constexpr auto FogTile =
        Tile{{'#', cxxg::types::RgbColor{20, 20, 20, true, 18, 18, 18}}};
    RenderedMap.fill(FogTile);
    return RenderedMap;
  }
};

class OverWorld : public GameWorld {
public:
  explicit OverWorld(ymir::Size2d<int> ChunkSize) : ChunkSize(ChunkSize) {}

  void initialize(ymir::Point2d<int> InitPos = {0, 0}) {
    loadSurroundingChunks(InitPos);
  }

  bool update(ymir::Point2d<int> Pos) {
    loadSurroundingChunks(Pos);
    // TODO unload chunks
    // TODO update chunks
    return true;
  }

  void loadSurroundingChunks(ymir::Point2d<int> Pos) {
    const auto ChunkPos = getChunkPos(Pos);
    loadChunk(ChunkPos);
    for (auto Pos : ymir::EightTileDirections<int>::get()) {
      Pos.X *= ChunkSize.W;
      Pos.Y *= ChunkSize.H;
      Pos += ChunkPos;
      loadChunk(Pos);
    }
  }

  void loadChunk(ymir::Point2d<int> ChunkPos) {
    auto It = ChunkMap.lower_bound(ChunkPos);
    if (It == ChunkMap.end() || It->first != ChunkPos) {
      std::cerr << "load chunk: " << ChunkPos << std::endl;
      auto Chunk = generateChunk(ChunkMap.size(), {ChunkPos, ChunkSize});
      ChunkMap.insert(It, {ChunkPos, Chunk});
    }
  }

  ymir::Map<Tile> renderMap(ymir::Rect2d<int> VisibleRect) const final {
    auto RenderedMap = setupRenderMap(VisibleRect.Size);

    auto Center = VisibleRect.Pos;
    Center.X += VisibleRect.Size.W / 2;
    Center.Y += VisibleRect.Size.H / 2;

    // for chunk in reach
    const auto ChunkPos = getChunkPos(Center);
    renderMapFromChunk(ChunkPos, VisibleRect, RenderedMap);
    for (auto Pos : ymir::EightTileDirections<int>::get()) {
      Pos.X *= ChunkSize.W;
      Pos.Y *= ChunkSize.H;
      Pos += ChunkPos;
      renderMapFromChunk(Pos, VisibleRect, RenderedMap);
    }

    return RenderedMap;
  }

  void renderMapFromChunk(ymir::Point2d<int> ChunkPos,
                          ymir::Rect2d<int> VisibleRect,
                          ymir::Map<Tile> &RenderedMap) const {
    const auto &Chunk = ChunkMap.at(ChunkPos);
    const auto ChunkRect = ymir::Rect2d<int>(ChunkPos, ChunkSize);
    auto ChunkVisibleRect = VisibleRect & ChunkRect;
    ChunkVisibleRect.Pos -= ChunkPos;
    if (ChunkVisibleRect.Size == ymir::Size2d<int>{0, 0}) {
      return;
    }

    const auto ChunkOffset = ChunkPos - VisibleRect.Pos;
    Chunk->Map.render().forEach(
        [ChunkOffset, &RenderedMap](auto Pos, auto Tile) {
          auto RMPos = Pos + ChunkOffset;
          RenderedMap.setTile(RMPos, Tile);
        },
        ChunkVisibleRect);
  }

  ymir::Point2d<int> getChunkPos(ymir::Point2d<int> Pos) const {
    ymir::Point2d<int> ChunkPos;
    ChunkPos.X = (Pos.X / ChunkSize.W) * ChunkSize.W;
    if (Pos.X < 0) {
      ChunkPos.X -= ChunkSize.W;
    }
    ChunkPos.Y = (Pos.Y / ChunkSize.H) * ChunkSize.H;
    if (Pos.Y < 0) {
      ChunkPos.Y -= ChunkSize.H;
    }
    return ChunkPos;
  }

  const ymir::Size2d<int> ChunkSize;
  std::map<ymir::Point2d<int>, std::shared_ptr<Level>> ChunkMap;
};

ymir::Map<cxxg::types::ColoredChar> renderWorldMap(ymir::Size2d<int> Size,
                                                   const GameWorld &GW,
                                                   ymir::Point2d<int> Center) {
  ymir::Map<cxxg::types::ColoredChar> VisibleMap(Size);
  ymir::Point2d<int> Offset;
  Offset.X = Center.X - Size.W / 2;
  Offset.Y = Center.Y - Size.H / 2;

  auto RenderedLevelMap = GW.renderMap({Offset, Size});

  VisibleMap.forEach([RenderedLevelMap](auto Pos, auto &Tile) {
    Tile = RenderedLevelMap.getTile(Pos).T;
  });
  return VisibleMap;
}

class ProcExplorer : public cxxg::Game {
public:
  using cxxg::Game::Game;

  void initialize(bool BufferedInput = false, unsigned TickDelayUs = 0) final {
    CurrentGameWorld = std::make_shared<OverWorld>(ChunkSize);
    CurrentGameWorld->initialize(GblPos);
    cxxg::Game::initialize(BufferedInput, TickDelayUs);
    handleDraw();
  }

  bool handleInput(int Char) final {
    switch (Char) {
    case 'a':
    case cxxg::utils::KEY_LEFT:
      movePlayer(ymir::Dir2d::LEFT);
      break;
    case 'd':
    case cxxg::utils::KEY_RIGHT:
      movePlayer(ymir::Dir2d::RIGHT);
      break;
    case 's':
    case cxxg::utils::KEY_DOWN:
      movePlayer(ymir::Dir2d::DOWN);
      break;
    case 'w':
    case cxxg::utils::KEY_UP:
      movePlayer(ymir::Dir2d::UP);
      break;
    case 'q':
      GameRunning = false;
      break;
    case '+':
      Speed += 1;
      break;
    case '-':
      Speed -= 1;
      Speed = Speed < 1 ? 1 : Speed;
      break;
    default:
      return false;
    }
    CurrentGameWorld->update(GblPos);
    return true;
  }

  void handleDraw() {
    const auto RenderSize = ymir::Size2d<int>{200, 50};
    auto VM = renderWorldMap(RenderSize, *CurrentGameWorld, GblPos);
    Scr << VM;

    Scr[0][0] << GblPos << ", Speed = " << Speed;
    if (const auto *OW =
            dynamic_cast<const OverWorld *>(CurrentGameWorld.get())) {
      Scr[1][0] << "[Chunk]: " << OW->getChunkPos(GblPos)
                << ", [NUM CHUNKS]: " << OW->ChunkMap.size();
    }
    Scr[RenderSize.H / 2][RenderSize.W / 2] << "x";
    cxxg::Game::handleDraw();
  }

  void movePlayer(ymir::Dir2d Dir) {
    for (int Num = 0; Num < Speed; Num++) {
      GblPos += Dir;
    }
  }

private:
  ymir::Point2d<int> GblPos;
  int Speed = 1;

  const ymir::Size2d<int> ChunkSize = {256, 64};
  std::shared_ptr<GameWorld> CurrentGameWorld;
};

int main(int Argc, char *Argv[]) {
  if (Argc != 1) {
    std::cerr << "usage: " << Argv[0] << std::endl;
    return 1;
  }

  cxxg::Screen Scr(cxxg::Screen::getTerminalSize());
  cxxg::utils::registerSigintHandler([]() { exit(0); });
  ProcExplorer ProcExp(Scr);

  ProcExp.initialize();
  ProcExp.run();

  return 0;
}