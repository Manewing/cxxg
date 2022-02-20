#include "NPCEntity.h"
#include "Level.h"
#include <ymir/Algorithm/Dijkstra.hpp>

std::ostream &operator<<(std::ostream &Out, const PhysState &PS) {
  Out << "PhysState{Thirst=" << PS.Thirst << ", Hunger=" << PS.Hunger
      << ", Fatigue=" << PS.Fatigue << "}";
  return Out;
}

const char *getActionStateStr(ActionState AS) {
  switch (AS) {
  case ActionState::IDLE:
    return "Idle";
  case ActionState::WANDER:
    return "Wander";
  case ActionState::SEARCH_DRINK:
    return "Search Drink";
  case ActionState::SEARCH_FOOD:
    return "Search Food";
  case ActionState::SLEEP:
    return "Sleep";
  }
  return "<invalid>";
}
std::ostream &operator<<(std::ostream &Out, ActionState AS) {
  Out << getActionStateStr(AS);
  return Out;
}

const char *getNeedKindStr(NeedKind Need) {
  switch (Need) {
  case NeedKind::NONE:
    return "None";
  case NeedKind::DRINK:
    return "Drink";
  case NeedKind::FOOD:
    return "Food";
  case NeedKind::SLEEP:
    return "Sleep";
  }
  return "<invalid>";
}
std::ostream &operator<<(std::ostream &Out, NeedKind Need) {
  Out << getNeedKindStr(Need);
  return Out;
}

void NPCEntity::update(Level &L) {
  updatePhysState();

  // 1. decide what todo
  decideAction();

  // 2. do it
  handleAction(L);
}

void NPCEntity::updatePhysState() {
  PS.Thirst -= 5;
  PS.Hunger -= 2;
  PS.Fatigue -= 1;
  // FIXME check > 0
}

void NPCEntity::decideAction() {
  // what needs are required to be fullfilled?
  auto Need = getBiggestNeed();
  auto NeedAction = getActionFromNeed(Need);

  // decide upon action based on need and current action
  // if (CurrentActionState == NeedAction) {
  //  return; // already working on biggest need
  //}
  switch (CurrentActionState) {
  case ActionState::IDLE:
  case ActionState::WANDER:
    if (NeedAction == ActionState::IDLE) {
      if (CurrentActionState == ActionState::WANDER) {
        CurrentActionState = ActionState::IDLE;
      } else {
        CurrentActionState = ActionState::WANDER;
      }
    } else {
      CurrentActionState = NeedAction;
    }
    break;
  case ActionState::SEARCH_FOOD:
    if (NeedAction == ActionState::SEARCH_DRINK) {
      CurrentActionState = NeedAction;
    }
    break;
  case ActionState::SEARCH_DRINK:
    break;
  case ActionState::SLEEP:
    // can't sleep if hungry or thirsty
    CurrentActionState = NeedAction;
    break;
  }
}

NeedKind NPCEntity::getBiggestNeed() const {
  NeedKind Need = NeedKind::NONE;
  if (PS.Fatigue < 1000) {
    Need = NeedKind::SLEEP;
  }
  if (PS.Hunger < 1000) {
    Need = NeedKind::FOOD;
  }
  if (PS.Thirst < 1000) {
    Need = NeedKind::DRINK;
  }
  return Need;
}

ActionState NPCEntity::getActionFromNeed(NeedKind Need) {
  switch (Need) {
  case NeedKind::NONE:
    return ActionState::IDLE;
  case NeedKind::SLEEP:
    return ActionState::SLEEP;
  case NeedKind::FOOD:
    return ActionState::SEARCH_FOOD;
  case NeedKind::DRINK:
    return ActionState::SEARCH_DRINK;
  }
  return ActionState::IDLE;
}

void NPCEntity::handleAction(Level &L) {
  switch (CurrentActionState) {
  case ActionState::IDLE:
    // nothing todo
    break;
  case ActionState::WANDER:
    wander(L);
    break;
  case ActionState::SEARCH_DRINK:
    searchObject(L, Tile{{'~'}}, [this](auto) {
      PS.Thirst += 750;
      CurrentActionState = ActionState::IDLE;
    });
    break;
  case ActionState::SEARCH_FOOD:
    // TODO check if has food in inventory
    // consume food in inventory avoid search if possible
    searchObject(L, Tile{{'#'}}, [this](auto) {
      PS.Hunger += 750;
      CurrentActionState = ActionState::IDLE;
    });
    break;
  case ActionState::SLEEP:
    if (--SearchCooldown == 0) {
      PS.Fatigue += 750;
      SearchCooldown = 5;
      CurrentActionState = ActionState::IDLE;
    }
    break;
  }
}

void NPCEntity::searchObject(
    Level &L, Tile T, std::function<void(ymir::Point2d<int>)> FoundCallback) {
  // FIXME do this once at setup of search
  auto DM = L.getDijkstraMap(T, Level::LayerObjectsIdx);
  auto PathToObject = ymir::Algorithm::getPathFromDijkstraMap(
      DM, Pos, ymir::FourTileDirections<int>());
  if (PathToObject.empty()) {
    // FIXME add no path found exception
    throw MovementBlockedException({-1, -1});
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
