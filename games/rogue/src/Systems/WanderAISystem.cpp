#include <rogue/Components/AI.h>
#include <rogue/Components/RaceFaction.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/Event.h>
#include <rogue/History.h>
#include <rogue/Level.h>
#include <rogue/Systems/WanderAISystem.h>
#include <ymir/Algorithm/Dijkstra.hpp>
#include <ymir/Algorithm/LineOfSight.hpp>
#include <ymir/Noise.hpp>

// FIXME move this, also should this be based on the level seed?
static std::random_device RandomEngine;

namespace rogue {

WanderAISystem::WanderAISystem(Level &L) : System(L.Reg), L(L) {}

void WanderAISystem::update(UpdateType Type) {
  if (Type == UpdateType::NoTick) {
    return;
  }

  auto View = Reg.view<PositionComp, WanderAIComp>();
  View.each([this](const auto &Entity, auto &Pos, auto &AI) {
    updateEntity(Entity, Pos, AI);
  });
}

void WanderAISystem::updateEntity(entt::entity Entity, PositionComp &PC,
                                  WanderAIComp &AI) {
  switch (AI.State) {
  case WanderAIState::Idle: {
    if (auto [TargetEt, LOS, FC] = checkForTarget(Entity, PC, AI);
        TargetEt != entt::null) {
      break;
    }

    if (AI.IdleDelayLeft-- == 0) {
      AI.State = WanderAIState::Wander;
      AI.IdleDelayLeft = AI.IdleDelay;
    }
  } break;
  case WanderAIState::Wander: {
    if (auto [TargetEt, LOS, FC] = checkForTarget(Entity, PC, AI);
        TargetEt != entt::null) {
      break;
    }

    AI.State = WanderAIState::Wander;

    auto NextPos = wander(PC);
    MovementComp MC;
    MC.Dir = ymir::Dir2d::fromMaxComponent(NextPos - PC.Pos);
    Reg.emplace<MovementComp>(Entity, MC);
  } break;

  case WanderAIState::Search: {
    // If lost agro stop the search
    if (AI.SearchDurationLeft-- == 0) {
      AI.State = WanderAIState::Wander;
      AI.LastTargetPos = std::nullopt;
      AI.SearchDurationLeft = 0;
      break;
    }

    // If in range chase otherwise continue to last seen pos
    auto [TargetEt, LOS, FC] = checkForTarget(Entity, PC, AI);
    std::optional<ymir::Point2d<int>> NextPosOrNone;
    if (TargetEt != entt::null) {
      NextPosOrNone = chaseTarget(TargetEt, PC, *LOS);
    } else {
      assert(AI.LastTargetPos && "No last target pos for wander system");
      NextPosOrNone = findPathToPoint(*AI.LastTargetPos, *AI.LastTargetPos, PC,
                                      LOS->LOSRange);
    }
    if (!NextPosOrNone) {
      break;
    }

    MovementComp MC;
    MC.Dir = ymir::Dir2d::fromMaxComponent(*NextPosOrNone - PC.Pos);
    Reg.emplace<MovementComp>(Entity, MC);
    break;
  }
  case WanderAIState::Chase: {
    // If in range stay, otherwise chase
    auto [TargetEt, LOS, FC] = checkForTarget(Entity, PC, AI);
    if (TargetEt == entt::null) {
      break;
    }

    auto NextPosOrNone = chaseTarget(TargetEt, PC, *LOS);
    if (!NextPosOrNone) {
      break;
    }
    MovementComp MC;
    MC.Dir = ymir::Dir2d::fromMaxComponent(*NextPosOrNone - PC.Pos);
    Reg.emplace<MovementComp>(Entity, MC);
  } break;

  default:
    assert(false && "Should never be reached");
    break;
  }

  // Always set movement component to consume AP
  if (!Reg.any_of<MovementComp>(Entity)) {
    MovementComp MC;
    MC.Dir = ymir::Dir2d::NONE;
    Reg.emplace<MovementComp>(Entity, MC);
  }
}

std::tuple<entt::entity, const LineOfSightComp *, const FactionComp *>
WanderAISystem::checkForTarget(entt::entity Entity, PositionComp &PC,
                               WanderAIComp &AI) {
  auto [TargetEt, LOS, FC] = findTarget(Entity, PC);

  // Deal with case we lost the target
  if (TargetEt == entt::null) {

    // Check if we changed the state
    if (AI.State == WanderAIState::Chase) {
      LostTargetEvent LTE;
      LTE.Entity = Entity;
      LTE.Registry = &Reg;
      publish(LTE);

      AI.State = WanderAIState::Search;
    }

    return {TargetEt, LOS, FC};
  }

  // Check if we changed the state
  if (AI.State != WanderAIState::Chase) {
    DetectTargetEvent DTE;
    DTE.Entity = Entity;
    DTE.Target = TargetEt;
    DTE.Registry = &Reg;
    publish(DTE);
  }
  AI.State = WanderAIState::Chase;
  AI.LastTargetPos = Reg.get<PositionComp>(TargetEt).Pos;
  AI.SearchDurationLeft = AI.SearchDuration;
  return {TargetEt, LOS, FC};
}

std::tuple<entt::entity, const LineOfSightComp *, const FactionComp *>
WanderAISystem::findTarget(entt::entity Entity,
                           const ymir::Point2d<int> &AtPos) {
  auto LOSComp = Reg.try_get<LineOfSightComp>(Entity);
  auto FacComp = Reg.try_get<FactionComp>(Entity);
  if (!LOSComp || !FacComp) {
    return {entt::null, nullptr, nullptr};
  }

  // FIMXE only finds single target, no ordering of distance
  entt::entity TargetEt = entt::null;
  ymir::Algorithm::traverseLOS(
      [this, &TargetEt, &FacComp](auto Pos) {
        if (auto T = L.getEntityAt(Pos); T != entt::null) {
          if (auto TFac = Reg.try_get<FactionComp>(T);
              TFac && FacComp->Faction != TFac->Faction) {
            TargetEt = T;
            // Target blocks LOS
            return true;
          }
        }
        return !L.isLOSBlocked(Pos);
      },
      AtPos, LOSComp->LOSRange, 0.3);

  return {TargetEt, LOSComp, FacComp};
}

ymir::Point2d<int> WanderAISystem::wander(const ymir::Point2d<int> AtPos) {
  auto AllNextPos = L.getAllNonBodyBlockedPosNextTo(AtPos);
  auto It =
      ymir::randomIterator(AllNextPos.begin(), AllNextPos.end(), RandomEngine);
  if (AllNextPos.empty() || It == AllNextPos.end()) {
    return AtPos;
  }
  return *It;
}

std::optional<ymir::Point2d<int>> WanderAISystem::findPathToPoint(
    const ymir::Point2d<int> ToPos, const ymir::Point2d<int> FutureToPos,
    const ymir::Point2d<int> AtPos, const unsigned LOSRange) {
  if (static_cast<unsigned>((ToPos - AtPos).length()) > LOSRange) {
    return std::nullopt;
  }
  ymir::Size2d<int> DMSize(LOSRange * 2 + 1, LOSRange * 2 + 1);
  ymir::Point2d<int> DMMidPos(LOSRange + 1, LOSRange + 1);

  // FIXME avoid recomputing this every time
  auto DM = ymir::Algorithm::getDijkstraMap(
      DMSize, DMMidPos,
      [this, &ToPos, &FutureToPos, &DMMidPos](auto Pos) {
        auto RealPos = Pos + FutureToPos - DMMidPos;
        if (RealPos == ToPos) {
          return false;
        }
        return L.isBodyBlocked(RealPos, /*Hard=*/true);
      },
      ymir::FourTileDirections<int>());

  std::uniform_int_distribution<int> OneZero(0, 1);
  auto PathToTarget = ymir::Algorithm::getPathFromDijkstraMap(
      DM, DMMidPos, AtPos - FutureToPos + DMMidPos,
      ymir::FourTileDirections<int>(), 1, OneZero(RandomEngine));

  // FIXME avoid recomputing this every time
  if (PathToTarget.empty()) {
    DM = ymir::Algorithm::getDijkstraMap(
        DMSize, DMMidPos,
        [this, &ToPos, &FutureToPos, &DMMidPos](auto Pos) {
          auto RealPos = Pos + FutureToPos - DMMidPos;
          if (RealPos == ToPos) {
            return false;
          }
          return L.isBodyBlocked(RealPos, /*Hard=*/false);
        },
        ymir::FourTileDirections<int>());

    std::uniform_int_distribution<int> OneZero(0, 1);
    PathToTarget = ymir::Algorithm::getPathFromDijkstraMap(
        DM, DMMidPos, AtPos - FutureToPos + DMMidPos,
        ymir::FourTileDirections<int>(), 1, OneZero(RandomEngine));
  }
  if (PathToTarget.empty()) {
    publish(ErrorMessageEvent()
            << "can't find path from " << AtPos << " to " << FutureToPos);
    return AtPos;
  }
  auto TargetPos = PathToTarget.at(0) + FutureToPos - DMMidPos;

  if (!L.isBodyBlocked(TargetPos)) {
    return TargetPos;
  }
  return std::nullopt;
}

std::optional<ymir::Point2d<int>>
WanderAISystem::chaseTarget(entt::entity TargetEt,
                            const ymir::Point2d<int> AtPos,
                            const LineOfSightComp &LOS) {
  const auto &TPC = Reg.get<PositionComp>(TargetEt);
  auto TPos = TPC.Pos;
  if (ymir::FourTileDirections<int>::isNextTo(AtPos, TPos)) {
    return std::nullopt;
  }

  const auto &TMC = Reg.try_get<MovementComp>(TargetEt);
  if (TMC) {
    TPos += TMC->Dir;
  }

  if (ymir::FourTileDirections<int>::isNextTo(AtPos, TPos)) {
    return std::nullopt;
  }

  return findPathToPoint(TPC.Pos, TPos, AtPos, LOS.LOSRange);
}

} // namespace rogue