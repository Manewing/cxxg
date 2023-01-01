#include "Systems/WanderAISystem.h"
#include "Level.h"
#include <ymir/Algorithm/Dijkstra.hpp>
#include <ymir/Algorithm/LineOfSight.hpp>
#include <ymir/Noise.hpp>
#include "History.h"

// FIXME move this, also should this be based on the level seed?
static std::random_device RandomEngine;

void WanderAISystem::update() {
  auto View = Reg.view<PositionComp, WanderAIComp, AgilityComp>();
  View.each([this](const auto &Entity, auto &Pos, auto &AI, auto &Ag) {
    switch (AI.State) {
    case WanderAIState::Idle: {
      if (AI.IdleDelayLeft-- == 0) {
        AI.State = WanderAIState::Wander;
        AI.IdleDelayLeft = AI.IdleDelay;
      }
      if (checkForTarget(Entity, Pos)) {
        AI.State = WanderAIState::Chase;
      }
    } break;
    case WanderAIState::Wander: {
      // FIXME switch to Idle after random duration
      if (!Ag.trySpendAP(AI.MoveAPCost)) {
        break;
      }

      Pos = wander(Pos);
      AI.DistanceWalked += 1;

      if (checkForTarget(Entity, Pos)) {
        AI.State = WanderAIState::Chase;
      }
    } break;

    case WanderAIState::Chase: {
      // If in range stay, otherwise chase
      auto Target = checkForTarget(Entity, Pos);
      if (!Target) {
        AI.State = WanderAIState::Idle;
      }
      // FIXME check in range
      Pos = chaseTarget(Pos, *Target);
      AI.DistanceWalked += 1;
    } break;

    default:
      assert(false && "Should never be reached");
      break;
    }
  });
}

void WanderAISystem::updateEntityPosCache() {
  EntityPosCache.fill(entt::null);
  // FIXME only include those with factions for now
  auto View = Reg.view<PositionComp, FactionComp>();
  for (auto [Entity, Pos, Fac] : View.each()) {
    EntityPosCache.setTile(Pos, Entity);
  }
}

std::optional<entt::entity>
WanderAISystem::checkForTarget(const entt::entity &Entity,
                               const ymir::Point2d<int> &AtPos) {
  auto LOSComp = Reg.try_get<LineOfSightComp>(Entity);
  if (!LOSComp) {
    return std::nullopt;
  }

  // FIMXE only finds single target, no ordering of distance
  std::optional<entt::entity> Target;
  ymir::Algorithm::traverseLOS(
      [this, &Target](auto Pos) {
        if (auto T = EntityPosCache.getTile(Pos); T != entt::null) {
          Target = T;
          // Target blocks LOS
          return true;
        }
        return L.isLOSBlocked(Pos);
      },
      AtPos, LOSComp->LOSRange);

  return Target;
}

ymir::Point2d<int> WanderAISystem::wander(const ymir::Point2d<int> AtPos) {
  auto AllNextPos = L.getAllNonBodyBlockedPosNextTo(AtPos);
  auto It =
      ymir::randomIterator(AllNextPos.begin(), AllNextPos.end(), RandomEngine);
  if (It == AllNextPos.end()) {
    // FIXME this may actuall happen if there are many entities body blocking
    // each other...
    throw MovementBlockedException(AtPos);
  }
  return *It;
}

ymir::Point2d<int> WanderAISystem::chaseTarget(const ymir::Point2d<int> AtPos,
                                               const entt::entity &Target) {
  const auto &TPos = Reg.get<PositionComp>(Target);

  // FIXME avoid recomputing this every time
  auto DM = ymir::Algorithm::getDijkstraMap(
      EntityPosCache.getSize(), TPos.Pos,
      [this](auto Pos) { return L.isBodyBlocked(Pos); },
      ymir::FourTileDirections<int>());

  auto PathToTarget = ymir::Algorithm::getPathFromDijkstraMap(
      DM, AtPos, ymir::FourTileDirections<int>(), 1);
  auto TargetPos = PathToTarget.at(0);

  if (!L.isBodyBlocked(TargetPos)) {
    publish(DebugMessageEvent{{}, "Entity targeted"});
    return TargetPos;
  }
  return AtPos;
}