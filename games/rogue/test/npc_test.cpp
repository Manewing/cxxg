#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <entt/entt.hpp>
#include <memory>
#include <rogue/Components/AI.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Helpers.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/RaceFaction.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/Visual.h>
#include <rogue/Context.h>
#include <rogue/CreatureDatabase.h>
#include <rogue/ItemDatabase.h>
#include <rogue/LevelGenerator.h>
#include <rogue/Renderer.h>
#include <ymir/LayeredMap.hpp>
#include <ymir/Map.hpp>

using namespace rogue;

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

using NPCCompList =
    ComponentList<TileComp, FactionComp, PositionComp, StatsComp, HealthComp,
                  NameComp, LineOfSightComp, AgilityComp, MeleeAttackComp,
                  InventoryComp, EquipmentComp, CollisionComp>;

entt::entity createNPCEntity(entt::registry &Reg, ymir::Point2d<int> Pos,
                             StatPoints Stats) {
  auto Entity = Reg.create();
  Reg.emplace<PositionComp>(Entity, Pos);
  Reg.emplace<PhysState>(Entity);
  Reg.emplace<ReasoningStateComp>(Entity);
  Reg.emplace<TileComp>(Entity, NPCTile);
  Reg.emplace<NameComp>(Entity, "NPC");
  Reg.emplace<AgilityComp>(Entity);
  Reg.emplace<StatsComp>(Entity, Stats);
  Reg.emplace<HealthComp>(Entity);
  Reg.emplace<FactionComp>(Entity, FactionKind::Player);
  Reg.emplace<RaceComp>(Entity, RaceKind::Human);
  Reg.emplace<LineOfSightComp>(Entity);
  Reg.emplace<MeleeAttackComp>(Entity);
  Reg.emplace<InventoryComp>(Entity);
  Reg.emplace<EquipmentComp>(Entity);
  Reg.emplace<CollisionComp>(Entity);

  assert(NPCCompList::validate(Reg, Entity) && "NPC entity is invalid");

  return Entity;
}

int main(int Argc, char *Argv[]) {
  if (Argc != 2) {
    std::cerr << "usage: " << Argv[0] << " <level_file>" << std::endl;
    return 1;
  }

  cxxg::Screen Scr(cxxg::Screen::getTerminalSize());
  cxxg::utils::registerSigintHandler([]() { exit(0); });

  rogue::ItemDatabase ItemDb;
  rogue::CreatureDatabase CreatureDb;
  rogue::GameContext Ctx{ItemDb, CreatureDb};

  LevelGeneratorLoader LvlGenLoader(Ctx);
  auto LG = LvlGenLoader.load(0, Argv[1]);
  auto Level = LG->generateLevel(0);
  auto NPCEntity =
      createNPCEntity(Level->Reg, {4, 4}, StatPoints{10, 10, 10, 10});

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