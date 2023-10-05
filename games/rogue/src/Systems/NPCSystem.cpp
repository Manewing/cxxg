#include <rogue/Components/Transform.h>
#include <rogue/Level.h>
#include <rogue/Systems/NPCSystem.h>
#include <ymir/Algorithm/Dijkstra.hpp>

namespace rogue {

void NPCSystem::decideAction(const PhysState &PS, ReasoningStateComp &RSC) {
  // what needs are required to be fullfilled?
  auto Need = PS.getBiggestNeed();
  auto NeedAction = getActionFromNeed(Need);

  // decide upon action based on need and current action
  // if (RSC.State == NeedAction) {
  //  return; // already working on biggest need
  //}
  switch (RSC.State) {
  case ActionState::IDLE:
  case ActionState::WANDER:
    if (NeedAction == ActionState::IDLE) {
      if (RSC.State == ActionState::WANDER) {
        RSC.State = ActionState::IDLE;
      } else {
        RSC.State = ActionState::WANDER;
      }
    } else {
      RSC.State = NeedAction;
    }
    break;
  case ActionState::SEARCH_FOOD:
    if (NeedAction == ActionState::SEARCH_DRINK) {
      RSC.State = NeedAction;
    }
    break;
  case ActionState::SEARCH_DRINK:
    break;
  case ActionState::SLEEP:
    // can't sleep if hungry or thirsty
    RSC.State = NeedAction;
    break;
  }
}

void NPCSystem::handleAction(Level &L, PhysState &PS, PositionComp &PC,
                             ReasoningStateComp &RSC) {
  // FIXME
  // REFACTOR
  int SearchCooldown = 5;

  switch (RSC.State) {
  case ActionState::IDLE:
    // nothing todo
    break;
  case ActionState::WANDER:
    // wander(L);
    //  FIXME
    // REFACTOR
    break;
  case ActionState::SEARCH_DRINK:
    searchObject(L, PC.Pos, Tile{{'~'}}, [&PS, &RSC](auto) {
      PS.Thirst += 750;
      RSC.State = ActionState::IDLE;
    });
    break;
  case ActionState::SEARCH_FOOD:
    // TODO check if has food in inventory
    // consume food in inventory avoid search if possible
    searchObject(L, PC.Pos, Tile{{'#'}}, [&PS, &RSC](auto) {
      PS.Hunger += 750;
      RSC.State = ActionState::IDLE;
    });
    break;
  case ActionState::SLEEP:
    if (--SearchCooldown == 0) {
      PS.Fatigue += 750;
      SearchCooldown = 5;
      RSC.State = ActionState::IDLE;
    }
    break;
  }
}

void NPCSystem::searchObject(
    Level &L, ymir::Point2d<int> &Pos, Tile T,
    std::function<void(ymir::Point2d<int>)> FoundCallback) {
  // FIXME
  // REFACTOR

  // FIXME do this once at setup of search
  auto [DM, TPs] = L.getDijkstraMap(T, Level::LayerObjectsIdx);
  auto PathToObject = ymir::Algorithm::getPathFromDijkstraMap(
      DM, TPs.at(0), Pos, ymir::FourTileDirections<int>());
  if (PathToObject.empty()) {
    // FIXME add no path found exception
    throw std::runtime_error("Could not find path to object");
  }
  auto TargetPos = PathToObject.at(0);
  auto EndPos = PathToObject.back();

  // Check if object is in reach
  if (TargetPos == EndPos) {
    FoundCallback(TargetPos);
    return;
  }

  if (!L.isBodyBlocked(TargetPos)) {
    Pos = PathToObject.at(0);
  } else {
    // FIXME should never be reached
  }
}

NPCSystem::NPCSystem(Level &L) : System(L.Reg), L(L) {}

void NPCSystem::update(UpdateType Type) {
  if (Type == UpdateType::NoTick) {
    return;
  }

  // Update physical state
  Reg.view<PhysState>().each([](auto &PS) { PS.update(); });

  // 1. decide what todo
  Reg.view<PhysState, ReasoningStateComp>().each(
      [](auto &PS, auto &RSC) { decideAction(PS, RSC); });

  // 2. do it
  Reg.view<PhysState, PositionComp, ReasoningStateComp>().each(
      [this](auto &PS, auto &PC, auto &RSC) { handleAction(L, PS, PC, RSC); });
}

} // namespace rogue