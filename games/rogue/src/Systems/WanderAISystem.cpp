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
    auto [TargetEt, LOS, FC] = checkForTarget(Entity, PC);
    if (TargetEt != entt::null) {
      AI.State = WanderAIState::Chase;
      break;
    }

    if (AI.IdleDelayLeft-- == 0) {
      AI.State = WanderAIState::Wander;
      AI.IdleDelayLeft = AI.IdleDelay;
    }
  } break;
  case WanderAIState::Wander: {
    auto [TargetEt, LOS, FC] = checkForTarget(Entity, PC);
    if (TargetEt != entt::null) {
      AI.State = WanderAIState::Chase;
      break;
    }

    auto NextPos = wander(PC);
    MovementComp MC;
    MC.Dir = ymir::Dir2d::fromMaxComponent(NextPos - PC.Pos);
    Reg.emplace<MovementComp>(Entity, MC);
  } break;

  case WanderAIState::Chase: {
    // If in range stay, otherwise chase
    auto [TargetEt, LOS, FC] = checkForTarget(Entity, PC);
    if (TargetEt == entt::null) {
      AI.State = WanderAIState::Idle;
      AI.IdleDelayLeft = AI.IdleDelay;
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
}

std::tuple<entt::entity, const LineOfSightComp *, const FactionComp *>
WanderAISystem::checkForTarget(entt::entity Entity,
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
      AtPos, LOSComp->LOSRange);

  return {TargetEt, LOSComp, FacComp};
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

std::optional<ymir::Point2d<int>> WanderAISystem::chaseTarget(entt::entity TargetEt,
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

  // FIXME avoid recomputing this every time
  unsigned LOSRange = LOS.LOSRange;
  ymir::Size2d<int> DMSize(LOSRange * 2 + 1, LOSRange * 2 + 1);
  ymir::Point2d<int> DMMidPos(LOSRange + 1, LOSRange + 1);
  auto DM = ymir::Algorithm::getDijkstraMap(
      DMSize, DMMidPos,
      [this, &TPC, &TPos, &DMMidPos](auto Pos) {
        auto RealPos = Pos + TPos - DMMidPos;
        if (RealPos == TPC.Pos) {
          return false;
        }
        return L.isBodyBlocked(RealPos);
      },
      ymir::FourTileDirections<int>());

  std::uniform_int_distribution<int> OneZero(0, 1);
  auto PathToTarget = ymir::Algorithm::getPathFromDijkstraMap(
      DM, DMMidPos, AtPos - TPos + DMMidPos, ymir::FourTileDirections<int>(), 1,
      OneZero(RandomEngine));
  if (PathToTarget.empty()) {
    publish(ErrorMessageEvent()
            << "can't find path from " << AtPos << " to " << TPos);
    return AtPos;
  }
  auto TargetPos = PathToTarget.at(0) + TPos - DMMidPos;

  if (!L.isBodyBlocked(TargetPos)) {
    return TargetPos;
  }
  return std::nullopt;
}

} // namespace rogue