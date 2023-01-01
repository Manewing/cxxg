#include "Level.h"
#include <ymir/Algorithm/Dijkstra.hpp>
#include <ymir/Algorithm/LineOfSight.hpp>

#include "Components/Level.h"
#include "Components/Transform.h"

Level::Level(const std::vector<std::string> &Layers, ymir::Size2d<int> Size)
    : Map(Layers, Size), AgSys(Reg), WAISys(*this, Reg), DeathSys(Reg),
      PlayerDijkstraMap(Size), PlayerSeenMap(Size) {
  PlayerSeenMap.fill(false);
}

void Level::setEventHub(EventHub *Hub) {
  EventHubConnector::setEventHub(Hub);
  DeathSys.setEventHub(Hub);
  WAISys.setEventHub(Hub);
  for (auto &E : Entities) {
    E->setEventHub(Hub);
  }
}

bool Level::update() {
  updatePlayerDijkstraMap();
  updatePlayerSeenMap();

  AgSys.update();
  WAISys.update();
  DeathSys.update();

  auto AllEntities = getEntities();
  std::sort(
      AllEntities.begin(), AllEntities.end(),
      [](const auto &A, const auto &B) { return A->Agility > B->Agility; });
  for (auto &Entity : AllEntities) {
    Entity->update(*this);
  }

  // Remove dead enemies
  auto DelIt =
      std::remove_if(Entities.begin(), Entities.end(),
                     [](const auto &Entity) { return !Entity->isAlive(); });
  Entities.erase(DelIt, Entities.end());

  if (Player) {
    return Player->isAlive();
  }
  return true;
}

void Level::setPlayer(PlayerEntity *P) { this->Player = P; }

PlayerEntity *Level::getPlayer() { return Player; }

ymir::Point2d<int> Level::getPlayerStartPos() const {
  std::optional<ymir::Point2d<int>> FoundPos;
  auto View = Reg.view<const PositionComp, LevelStartComp>();
  View.each([&FoundPos](const auto &Pos) { FoundPos = Pos; });
  if (!FoundPos) {
    throw std::runtime_error("Could not find start in level");
  }
  auto Pos = getNonBodyBlockedPosNextTo(*FoundPos);
  if (!Pos) {
    throw std::runtime_error(
        "Could not find start position for player in level");
  }
  return *Pos;
}

ymir::Point2d<int> Level::getPlayerEndPos() const {
  std::optional<ymir::Point2d<int>> FoundPos;
  auto View = Reg.view<const PositionComp, LevelEndComp>();
  View.each([&FoundPos](const auto &Pos) { FoundPos = Pos; });
  if (!FoundPos) {
    throw std::runtime_error("Could not find end in level");
  }
  auto Pos = getNonBodyBlockedPosNextTo(*FoundPos);
  if (!Pos) {
    throw std::runtime_error("Could not find end position for player in level");
  }
  return *Pos;
}

std::vector<Entity *> Level::getEntities() {
  std::vector<Entity *> AllEntities;
  AllEntities.reserve(Entities.size() + 1);
  for (auto &Entity : Entities) {
    AllEntities.push_back(Entity.get());
  }
  if (Player) {
    AllEntities.push_back(Player);
  }
  return AllEntities;
}

Entity *Level::getEntityAt(ymir::Point2d<int> Pos) {
  if (Player && Player->Pos == Pos) {
    return Player;
  }
  auto It =
      std::find_if(Entities.begin(), Entities.end(),
                   [Pos](const auto &Entity) { return Entity->Pos == Pos; });
  if (It == Entities.end()) {
    return nullptr;
  }
  return It->get();
}

std::vector<ymir::Point2d<int>>
Level::getAllNonBodyBlockedPosNextTo(ymir::Point2d<int> AtPos) const {
  std::vector<ymir::Point2d<int>> AllUnblockedPos;
  // FIXME not actually using the map data here, only the neighbor check func
  Map.get(LayerWallsIdx)
      .checkNeighbors(
          AtPos,
          [this, &AllUnblockedPos](auto Pos, auto) {
            if (!isBodyBlocked(Pos)) {
              AllUnblockedPos.push_back(Pos);
            }
            return true;
          },
          ymir::FourTileDirections<int>());
  return AllUnblockedPos;
}

std::optional<ymir::Point2d<int>>
Level::getNonBodyBlockedPosNextTo(ymir::Point2d<int> AtPos) const {
  auto AllUnblockedPos = getAllNonBodyBlockedPosNextTo(AtPos);
  if (AllUnblockedPos.empty()) {
    return {};
  }
  return AllUnblockedPos.at(0);
}

bool Level::canInteract(ymir::Point2d<int> AtPos) const {
  bool FoundObject = false;
  (void)AtPos;
  /*
  Map.get(LayerObjectsIdx)
      .checkNeighbors(
          AtPos,
          [&FoundObject](auto, auto Tile) {
            if (Tile != EmptyTile) {
              FoundObject = true;
              return false;
            }
            return true;
          },
          ymir::FourTileDirections<int>());
          */
  return FoundObject;
}

std::vector<Tile> Level::getInteractables(ymir::Point2d<int> AtPos) const {
  std::vector<Tile> Objects;
  (void)AtPos;
  /*
  Objects.reserve(4);
  Map.get(LayerObjectsIdx)
      .checkNeighbors(
          AtPos,
          [&Objects](auto, auto Tile) {
            if (Tile != EmptyTile) {
              Objects.push_back(Tile);
            }
            return true;
          },
          ymir::FourTileDirections<int>());
  */
  return Objects;
}

bool Level::isLOSBlocked(ymir::Point2d<int> Pos) const {
  return Map.get(LayerWallsIdx).getTile(Pos) != EmptyTile;
}

bool Level::isBodyBlocked(ymir::Point2d<int> Pos) const {
  bool MapBlocked = Map.get(LayerWallsIdx).getTile(Pos) != EmptyTile;
  bool EntityBlocked =
      std::any_of(Entities.begin(), Entities.end(),
                  [Pos](const auto &Entity) { return Entity->Pos == Pos; });
  bool PlayerBlocked = Player && Player->Pos == Pos;
  return MapBlocked || EntityBlocked || PlayerBlocked;
}

ymir::Map<int, int> Level::getDijkstraMap(Tile Target,
                                          std::size_t Layer) const {
  auto &LayerMap = Map.get(Layer);
  auto TilePos = LayerMap.findTiles(Target);
  return ymir::Algorithm::getDijkstraMap(
      Map.getSize(), TilePos, [this](auto Pos) { return isBodyBlocked(Pos); },
      ymir::FourTileDirections<int>());
}

const ymir::Map<int, int> &Level::getPlayerDijkstraMap() const {
  return PlayerDijkstraMap;
}

const ymir::Map<bool, int> &Level::getPlayerSeenMap() const {
  return PlayerSeenMap;
}

void Level::updatePlayerDijkstraMap() {
  if (!Player) {
    PlayerDijkstraMap.fill(-1);
    return;
  }

  PlayerDijkstraMap = ymir::Algorithm::getDijkstraMap(
      Map.getSize(), Player->Pos,
      [this](auto Pos) { return isBodyBlocked(Pos); },
      ymir::FourTileDirections<int>());

  // auto MapCopy = L.Map.get(Level::LayerWallsIdx);
  // ymir::Map<ymir::ColoredUniChar, int> HM(MapCopy.getSize());
  // HM.forEach([&MapCopy](auto Pos, auto &Tile) {
  //  Tile = ymir::ColoredUniChar{MapCopy.getTile(Pos).kind()};
  //});
  // auto NewHM = ymir::Algorithm::makeHeatMap(HM, DM);
  // std::cerr << NewHM << std::endl;
}

void Level::updatePlayerSeenMap() {
  if (!Player) {
    return;
  }
  ymir::Algorithm::traverseLOS(
      [this](auto Pos) -> bool {
        if (!PlayerSeenMap.contains(Pos)) {
          return false;
        }
        PlayerSeenMap.getTile(Pos) = true;
        return !isLOSBlocked(Pos);
      },
      Player->Pos, Player->LOSRange, 0.01);
}
