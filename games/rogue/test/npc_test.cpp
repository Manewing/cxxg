#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <entt/entt.hpp>
#include <memory>
#include <rogue/Components/AI.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/LevelGenerator.h>
#include <rogue/Renderer.h>
#include <ymir/LayeredMap.hpp>
#include <ymir/Map.hpp>

using namespace rogue;

static const Tile GroundTile{
    {' ', cxxg::types::RgbColor{0, 0, 0, true, 100, 80, 50}}};
static const Tile WallTile{
    {'#', cxxg::types::RgbColor{60, 60, 60, true, 45, 45, 45}}};
static const Tile WaterTile{
    {'~', cxxg::types::RgbColor{40, 132, 191, true, 30, 110, 150}}};
static const Tile TreeTile{
    {'A', cxxg::types::RgbColor{10, 50, 15, true, 100, 80, 50}}};
static const Tile BerryBushTile{
    {'#', cxxg::types::RgbColor{140, 10, 50, true, 10, 50, 15}}};
static const Tile NPCTile{
    {'@', cxxg::types::RgbColor{0, 60, 255, true, 100, 80, 50}}};

template <typename T, typename U>
cxxg::Screen &operator<<(cxxg::Screen &Scr, const ymir::Map<T, U> &Map) {
  for (auto PY = 0; PY < Map.getSize().H; PY++) {
    for (auto PX = 0; PX < Map.getSize().W; PX++) {
      Scr[PY][PX] = Map.getTile({PX, PY});
    }
  }
  return Scr;
}

entt::entity createNPCEntity(entt::registry &Reg, ymir::Point2d<int> Pos) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<PhysState>(Entity);
  Reg.emplace<ReasoningStateComp>(Entity);
  Reg.emplace<TileComp>(Entity, NPCTile);
  Reg.emplace<NameComp>(Entity, "NPC");
  return Entity;
}

int main(int Argc, char *Argv[]) {
  if (Argc != 2) {
    std::cerr << "usage: " << Argv[0] << " <level_file>" << std::endl;
    return 1;
  }

  cxxg::Screen Scr(cxxg::Screen::getTerminalSize());
  cxxg::utils::registerSigintHandler([]() { exit(0); });

  LevelGenerator LG;

  const std::vector<std::string> Layers = {
      "ground", "ground_deco", "walls", "walls_deco", "objects",
  };
  const std::map<char, LevelGenerator::CharInfo> CharInfoMap = {
      {GroundTile.kind(), {GroundTile, "ground"}},
      {WallTile.kind(), {WallTile, "walls"}},
      {WaterTile.kind(), {WaterTile, "objects"}},
      {TreeTile.kind(), {TreeTile, "objects"}},
      {'B', {BerryBushTile, "objects"}},
  };
  auto Level = LG.loadLevel(Argv[1], Layers, CharInfoMap, 0);
  auto NPCEntity = createNPCEntity(Level->Reg, {4, 4});

  unsigned MaxTick = 1000;
  for (unsigned Tick = 0; Tick < MaxTick; Tick++) {

    const auto RenderSize = ymir::Size2d<int>{80, 24};
    Renderer Render(RenderSize, *Level, {40, 12});
    Scr << Render.get();

    auto &PC = Level->Reg.get<PositionComp>(NPCEntity);
    auto &PS = Level->Reg.get<PhysState>(NPCEntity);
    auto &RSC = Level->Reg.get<ReasoningStateComp>(NPCEntity);
    auto Need = PS.getBiggestNeed();
    Scr[0][0] << "NPC@" << PC.Pos << " " << PS;
    Scr[1][0] << "[" << Need << "][" << RSC.State << "/"
              << getActionFromNeed(Need) << "]";

    Scr.update();
    Scr.clear();
    Level->update(true);
    cxxg::utils::sleep(200000);
  }

  return 0;
}