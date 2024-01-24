#include <rogue/Components/AI.h>
#include <rogue/Components/LOS.h>
#include <rogue/Components/RaceFaction.h>
#include <rogue/Components/Transform.h>
#include <rogue/Event.h>
#include <rogue/Level.h>
#include <rogue/Systems/SearchAISystem.h>
#include <ymir/Algorithm/Dijkstra.hpp>
#include <ymir/Algorithm/LineOfSight.hpp>
#include <ymir/Noise.hpp>

// FIXME move this, also should this be based on the level seed?
static std::random_device RandomEngine;

namespace rogue {

SearchAISystem::SearchAISystem(Level &L) : System(L.Reg), L(L) {}

void SearchAISystem::update(UpdateType Type) {
  if (Type == UpdateType::NoTick) {
    return;
  }

  auto View = Reg.view<PositionComp, SearchAIComp>();
  View.each([this](const auto &Entity, auto &Pos, auto &AI) {
    updateEntity(Entity, Pos, AI);
  });
}

void SearchAISystem::updateEntity(entt::entity Entity, PositionComp &PC,
                                  SearchAIComp &AI) {
  // Always set movement component to consume AP
  auto *MC = Reg.try_get<MovementComp>(Entity);
  if (!MC) {
    MC = &Reg.emplace<MovementComp>(Entity);
    MC->Dir = ymir::Dir2d::NONE;
  }

  switch (AI.State) {
  case SearchAIState::Idle: {
    if (auto [TargetEt, LOS, FC] = checkForTarget(Entity, PC, AI);
        TargetEt != entt::null) {
      break;
    }
  } break;
  case SearchAIState::Search: {
    // If lost agro stop the search
    if (AI.SearchDurationLeft-- == 0) {
      AI.State = SearchAIState::Idle;
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

    MC->Dir = ymir::Dir2d::fromMaxComponent(*NextPosOrNone - PC.Pos);
    break;
  }
  case SearchAIState::Chase: {
    // If in range stay, otherwise chase
    auto [TargetEt, LOS, FC] = checkForTarget(Entity, PC, AI);
    if (TargetEt == entt::null) {
      break;
    }

    auto NextPosOrNone = chaseTarget(TargetEt, PC, *LOS);
    if (!NextPosOrNone) {
      break;
    }

    MC->Dir = ymir::Dir2d::fromMaxComponent(*NextPosOrNone - PC.Pos);
  } break;

  default:
    assert(false && "Should never be reached");
    break;
  }
}

std::tuple<entt::entity, const LineOfSightComp *, const FactionComp *>
SearchAISystem::checkForTarget(entt::entity Entity, PositionComp &PC,
                               SearchAIComp &AI) {
  auto [TargetEt, LOS, FC] = findTarget(Entity, PC);

  // Deal with case we lost the target
  if (TargetEt == entt::null) {

    // Check if we changed the state
    if (AI.State == SearchAIState::Chase) {
      LostTargetEvent LTE;
      LTE.Entity = Entity;
      LTE.Registry = &Reg;
      publish(LTE);

      AI.State = SearchAIState::Search;
    }

    return {TargetEt, LOS, FC};
  }

  // Check if we changed the state
  if (AI.State != SearchAIState::Chase) {
    DetectTargetEvent DTE;
    DTE.Entity = Entity;
    DTE.Target = TargetEt;
    DTE.Registry = &Reg;
    publish(DTE);
  }
  AI.State = SearchAIState::Chase;
  AI.LastTargetPos = Reg.get<PositionComp>(TargetEt).Pos;
  AI.SearchDurationLeft = AI.SearchDuration;
  return {TargetEt, LOS, FC};
}

std::tuple<entt::entity, const LineOfSightComp *, const FactionComp *>
SearchAISystem::findTarget(entt::entity Entity,
                           const ymir::Point2d<int> &AtPos) {
  auto LOSComp = Reg.try_get<LineOfSightComp>(Entity);
  auto FacComp = Reg.try_get<FactionComp>(Entity);
  if (!LOSComp || !FacComp) {
    return {entt::null, nullptr, nullptr};
  }

  // FIXME only finds single target, no ordering of distance
  entt::entity TargetEt = entt::null;
  Reg.view<PositionComp, FactionComp, VisibleComp>().each(
      [this, &TargetEt, &LOSComp, &FacComp, AtPos](
          entt::entity TEt, const auto &TPC, const auto &TFC, const auto &VC) {
        if (!VC.IsVisible || TFC.Faction == FacComp->Faction) {
          return;
        }

        if ((TPC.Pos - AtPos).length() > double(LOSComp->LOSRange)) {
          return;
        }
        auto const Offset = ymir::Point2d<double>(0.5, 0.5);
        ymir::Algorithm::rayCastDDA<int>(
            [&TargetEt, TPC, TEt](auto Pos) {
              if (TPC.Pos == Pos) {
                TargetEt = TEt;
              }
            },
            [this](auto Pos) { return L.isLOSBlocked(Pos); }, LOSComp->LOSRange,
            AtPos.template to<double>() + Offset,
            TPC.Pos.template to<double>() + Offset);
      });

  return {TargetEt, LOSComp, FacComp};
}

std::optional<ymir::Point2d<int>> SearchAISystem::findPathToPoint(
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
  // FIXME only consider this if the path is not significantly longer than one
  // taken if the entity was not body blocked
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
    // FIXME this can happen when the targeted entity moves out of range of the
    // LOS, this means that the dijkstra map is too small to be able to find a
    // path to the target. Could be avoided by switching to an A* algorithm.
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
SearchAISystem::chaseTarget(entt::entity TargetEt,
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