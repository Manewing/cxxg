#include <rogue/Components/Level.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Transform.h>
#include <rogue/Components/AI.h>
#include <rogue/Level.h>
#include <rogue/Systems/AgilitySystem.h>
#include <rogue/Systems/AttackAISystem.h>
#include <rogue/Systems/CombatSystem.h>
#include <rogue/Systems/DeathSystem.h>
#include <rogue/Systems/MovementSystem.h>
#include <rogue/Systems/PlayerSystem.h>
#include <rogue/Systems/RegenSystem.h>
#include <rogue/Systems/StatsSystem.h>
#include <rogue/Systems/WanderAISystem.h>
#include <ymir/Algorithm/Dijkstra.hpp>
#include <ymir/Algorithm/LineOfSight.hpp>

namespace rogue {

Level::Level(int LevelId, const std::vector<std::string> &Layers,
             ymir::Size2d<int> Size)
    : Map(Layers, Size), LevelId(LevelId), PlayerDijkstraMap(Size),
      PlayerSeenMap(Size) {
  Systems = {
      std::make_shared<StatsSystem>(Reg),
      std::make_shared<AgilitySystem>(Reg),
      std::make_shared<RegenSystem>(Reg),
      std::make_shared<PlayerSystem>(*this),
      std::make_shared<WanderAISystem>(*this),
      std::make_shared<AttackAISystem>(Reg),
      std::make_shared<CombatSystem>(Reg),
      std::make_shared<MovementSystem>(*this),
      std::make_shared<DeathSystem>(Reg),
  };
  PlayerSeenMap.fill(false);
}

void Level::setEventHub(EventHub *Hub) {
  EventHubConnector::setEventHub(Hub);
  for (auto &Sys : Systems) {
    Sys->setEventHub(Hub);
  }
  for (auto &E : Entities) {
    E->setEventHub(Hub);
  }
}

bool Level::update(bool IsTick) {
  updatePlayerDijkstraMap();
  updatePlayerSeenMap();
  updateEntityPosCache();

  for (auto &Sys : Systems) {
    Sys->update(IsTick ? System::UpdateType::Tick : System::UpdateType::NoTick);
  }

  return true;
}

void Level::createPlayer() {
  Player = PlayerComp::createPlayer(Reg, "Player", getPlayerStartPos());
}

void Level::movePlayer(Level &From, ymir::Point2d<int> ToPos) {
  Player = PlayerComp::movePlayer(From.Reg, Reg);
  From.Player = entt::null;
  auto &PC = Reg.get<PositionComp>(Player);
  PC.Pos = ToPos;
  updateEntityPosition(Player, PC, ToPos);
}

void Level::removePlayer() { PlayerComp::removePlayer(Reg); }

const entt::entity &Level::getPlayer() const { return Player; }

namespace {

template <typename LevelComp>
ymir::Point2d<int> getPlayerStartEndPos(const Level &L) {
  std::optional<ymir::Point2d<int>> FoundPos;
  auto View = L.Reg.view<const PositionComp, const LevelComp>();
  View.each([&FoundPos](const auto &Pos, const auto &) { FoundPos = Pos; });
  if (!FoundPos) {
    throw std::runtime_error("Could not find start/end in level");
  }
  auto Pos = L.getNonBodyBlockedPosNextTo(*FoundPos);
  if (!Pos) {
    throw std::runtime_error(
        "Could not find non-blocked start/end position for player in level");
  }
  return *Pos;
}

} // namespace

ymir::Point2d<int> Level::getPlayerStartPos() const {
  return getPlayerStartEndPos<LevelStartComp>(*this);
}

ymir::Point2d<int> Level::getPlayerEndPos() const {
  return getPlayerStartEndPos<LevelEndComp>(*this);
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
  auto View = Reg.view<const PositionComp, const InteractableComp>();
  for (auto [E, P, I] : View.each()) {
    if (ymir::FourTileDirections<int>::isNextTo(P, AtPos) || P.Pos == AtPos) {
      return true;
    }
  }
  return false;
}

std::vector<entt::entity>
Level::getInteractables(ymir::Point2d<int> AtPos) const {
  std::vector<entt::entity> Entities;
  Entities.reserve(5);

  auto View = Reg.view<const PositionComp, const InteractableComp>();
  View.each([AtPos, &Entities](const auto &Entity, const auto &P,
                               const auto &) {
    if (ymir::FourTileDirections<int>::isNextTo(P, AtPos) || P.Pos == AtPos) {
      Entities.push_back(Entity);
    }
  });

  return Entities;
}

bool Level::isWallBlocked(ymir::Point2d<int> Pos) const {
  if (!Map.contains(Pos)) {
    return true;
  }
  return Map.get(LayerWallsIdx).getTile(Pos) != EmptyTile;
}

bool Level::isLOSBlocked(ymir::Point2d<int> Pos) const {
  return isWallBlocked(Pos);
}

bool Level::isBodyBlocked(ymir::Point2d<int> Pos) const {
  if (!Map.contains(Pos)) {
    return true;
  }
  bool MapBlocked = Map.get(LayerWallsIdx).getTile(Pos) != EmptyTile;
  bool EntityBlocked = getEntityAt(Pos) != entt::null;
  return MapBlocked || EntityBlocked;
}

std::pair<ymir::Map<int, int>, std::vector<ymir::Point2d<int>>>
Level::getDijkstraMap(Tile Target, std::size_t Layer) const {
  auto &LayerMap = Map.get(Layer);
  auto TilePos = LayerMap.findTiles(Target);
  auto DM = ymir::Algorithm::getDijkstraMap(
      Map.getSize(), TilePos, [this](auto Pos) { return isBodyBlocked(Pos); },
      ymir::FourTileDirections<int>());
  return {DM, TilePos};
}

const ymir::Map<int, int> &Level::getPlayerDijkstraMap() const {
  return PlayerDijkstraMap;
}

const ymir::Map<bool, int> &Level::getPlayerSeenMap() const {
  return PlayerSeenMap;
}

const entt::entity &Level::getEntityAt(ymir::Point2d<int> AtPos) const {
  if (!EntityPosCache.contains(AtPos)) {
    static const entt::entity EtNull = entt::null;
    return EtNull;
  }
  return EntityPosCache.getTile(AtPos);
}

void Level::updateEntityPosition(const entt::entity &Entity,
                                 PositionComp &PosComp,
                                 const ymir::Point2d<int> NextPos) {
  const auto HasCollision = Reg.all_of<CollisionComp>(Entity);
  if (HasCollision) {
    EntityPosCache.setTile(NextPos, Entity);
  }
  if (PosComp.Pos == NextPos) {
    return;
  }
  if (HasCollision) {
    EntityPosCache.setTile(PosComp, entt::null);
  }
  PosComp.Pos = NextPos;
}

void Level::updatePlayerDijkstraMap() {
  if (Player == entt::null) {
    PlayerDijkstraMap.fill(-1);
    return;
  }

  PlayerDijkstraMap = ymir::Algorithm::getDijkstraMap(
      Map.getSize(), Reg.get<PositionComp>(Player).Pos,
      [this](auto Pos) { return isBodyBlocked(Pos); },
      ymir::FourTileDirections<int>());
}

void Level::updatePlayerSeenMap() {
  if (Player == entt::null) {
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
      Reg.get<PositionComp>(Player).Pos,
      Reg.get<LineOfSightComp>(Player).LOSRange, 0.1, 0.01);
}

void Level::updateEntityPosCache() {
  if (EntityPosCache.empty()) {
    EntityPosCache.resize(Map.getSize());
  }

  EntityPosCache.fill(entt::null);
  auto View = Reg.view<PositionComp, CollisionComp>();
  for (auto [Entity, Pos] : View.each()) {
    // FIXME check not overlapping
    EntityPosCache.setTile(Pos, Entity);
  }
}

} // namespace rogue