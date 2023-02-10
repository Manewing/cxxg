#include <rogue/History.h>
#include <rogue/Level.h>
#include <rogue/Systems/WanderAISystem.h>
#include <ymir/Algorithm/Dijkstra.hpp>
#include <ymir/Algorithm/LineOfSight.hpp>
#include <ymir/Noise.hpp>

// FIXME move this, also should this be based on the level seed?
static std::random_device RandomEngine;

namespace rogue {

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

      auto NextPos = wander(Pos);
      L.updateEntityPosition(Entity, Pos, NextPos);
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
        break;
      }
      // FIXME check in range
      auto NextPos = chaseTarget(Pos, *Target);
      L.updateEntityPosition(Entity, Pos, NextPos);
      AI.DistanceWalked += 1;
    } break;

    default:
      assert(false && "Should never be reached");
      break;
    }
  });
}

std::optional<entt::entity>
WanderAISystem::checkForTarget(const entt::entity &Entity,
                               const ymir::Point2d<int> &AtPos) {
  auto LOSComp = Reg.try_get<LineOfSightComp>(Entity);
  auto FacComp = Reg.try_get<FactionComp>(Entity);
  if (!LOSComp || !FacComp) {
    return std::nullopt;
  }

  // FIMXE only finds single target, no ordering of distance
  std::optional<entt::entity> Target;
  ymir::Algorithm::traverseLOS(
      [this, &Target, &FacComp](auto Pos) {
        if (auto T = L.getEntityAt(Pos); T != entt::null) {
          if (auto TFac = Reg.try_get<FactionComp>(T);
              TFac && FacComp->Faction != TFac->Faction) {
            Target = T;
            // Target blocks LOS
            return true;
          }
        }
        return !L.isLOSBlocked(Pos);
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
  unsigned LOSRange = 8; // FIXME get from LOS Comp
  ymir::Size2d<int> DMSize(LOSRange * 2 + 1, LOSRange * 2 + 1);
  ymir::Point2d<int> DMMidPos(LOSRange + 1, LOSRange + 1);
  auto DM = ymir::Algorithm::getDijkstraMap(
      DMSize, DMMidPos,
      [this, &TPos, &DMMidPos](auto Pos) {
        return L.isBodyBlocked(Pos + TPos.Pos - DMMidPos);
      },
      ymir::FourTileDirections<int>());

  auto PathToTarget = ymir::Algorithm::getPathFromDijkstraMap(
      DM, AtPos - TPos.Pos + DMMidPos, ymir::FourTileDirections<int>(), 1);
  if (PathToTarget.empty()) {
    publish(DebugMessageEvent()
            << "can't find path from " << AtPos << " to " << TPos.Pos);
    return AtPos;
  }
  auto TargetPos = PathToTarget.at(0) + TPos.Pos - DMMidPos;

  if (!L.isBodyBlocked(TargetPos)) {
    return TargetPos;
  }
  return AtPos;
}

} // namespace rogue