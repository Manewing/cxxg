#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <rogue/Context.h>
#include <rogue/CraftingHandler.h>
#include <rogue/EntityDatabase.h>
#include <rogue/GameConfig.h>
#include <rogue/ItemDatabase.h>
#include <rogue/LevelDatabase.h>
#include <rogue/LevelGenerator.h>
#include <rogue/RenderEventCollector.h>
#include <rogue/Renderer.h>
#include <ymir/LayeredMap.hpp>
#include <ymir/Map.hpp>

template <typename T, typename U>
cxxg::Screen &operator<<(cxxg::Screen &Scr, const ymir::Map<T, U> &Map) {
  for (auto PY = 0; PY < Map.getSize().H; PY++) {
    for (auto PX = 0; PX < Map.getSize().W; PX++) {
      Scr[PY][PX] = Map.getTile({PX, PY});
    }
  }
  return Scr;
}

void usage(const char *Argv0) {
  std::cerr << "usage: " << Argv0
            << " <game_config> <level_file> (--seed <value>)" << std::endl;
}

int main(int Argc, char *Argv[]) {
  if (Argc != 3 && Argc != 5) {
    usage(Argv[0]);
    return 1;
  }
  auto Cfg = rogue::GameConfig::load(Argv[1]);
  if (Argc == 5) {
    if (std::string(Argv[3]) != "--seed") {
      usage(Argv[0]);
      return 1;
    }
    Cfg.Seed = std::stoi(Argv[4]);
  } else {
    Cfg.Seed = std::time(nullptr);
  }
  std::srand(Cfg.Seed);

  cxxg::Screen Scr(cxxg::Screen::getTerminalSize());
  cxxg::utils::registerSigintHandler([]() { exit(0); });

  rogue::GeneratedMapLevelGenerator::DebugRooms = true;

  rogue::EventHub Hub;
  rogue::RenderEventCollector RenderCollector;
  RenderCollector.setEventHub(&Hub);
  auto ItemDb = rogue::ItemDatabase::load(Cfg.ItemDbConfig);
  auto EntityDb = rogue::EntityDatabase::load(ItemDb, Cfg.EntityDbConfig);
  auto LevelDb = rogue::LevelDatabase::load(Cfg.LevelDbConfig);
  auto CraftingDb = rogue::CraftingDatabase::load(ItemDb, Cfg.CraftingDbConfig);
  rogue::CraftingHandler Crafter(ItemDb);
  rogue::GameContext Ctx{Hub, ItemDb, EntityDb, LevelDb, CraftingDb, Crafter};

  rogue::LevelGeneratorLoader LvlGenLoader(Ctx);
  auto LG = LvlGenLoader.load(Cfg.Seed, Argv[2]);
  auto Level = LG->generateLevel(0);
  Level->setEventHub(&Hub);

  while (true) {
    // Render the current map
    const auto RenderSize = ymir::Size2d<int>{
        static_cast<int>(Scr.getSize().X), static_cast<int>(Scr.getSize().Y)};
    // Compute center position
    const auto CenterPos = ymir::Point2d<int>{Level->Map.getSize().W / 2,
                                              Level->Map.getSize().H / 2};
    rogue::Renderer Render(RenderSize, *Level, CenterPos);
    Render.renderEntities();
    RenderCollector.apply(Render);
    RenderCollector.clear();
    Scr << Render.get();

    Scr.update();
    Scr.clear();
    Level->update(true);
    cxxg::utils::sleep(200000);
  }
  return 0;
}