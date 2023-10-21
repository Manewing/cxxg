#include <iostream>
#include <rogue/Components/AI.h>

namespace rogue {

const char *getWanderAIStateStr(WanderAIState State) {
  switch (State) {
  case WanderAIState::Idle:
    return "Idle";
  case WanderAIState::Wander:
    return "Wander";
  case WanderAIState::Chase:
    return "Chase";
  case WanderAIState::Search:
    return "Search";
  }
  return "<invalid>";
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
  case NeedKind::SHELTER:
    return "Shelter";
  case NeedKind::CLOTHING:
    return "Clothing";
  }
  return "<invalid>";
}
std::ostream &operator<<(std::ostream &Out, NeedKind Need) {
  Out << getNeedKindStr(Need);
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

ActionState getActionFromNeed(NeedKind Need) {
  switch (Need) {
  case NeedKind::NONE:
    return ActionState::IDLE;
  case NeedKind::SLEEP:
    return ActionState::SLEEP;
  case NeedKind::FOOD:
    return ActionState::SEARCH_FOOD;
  case NeedKind::DRINK:
    return ActionState::SEARCH_DRINK;
  case NeedKind::SHELTER:
    // FIXME
    return ActionState::IDLE;
  case NeedKind::CLOTHING:
    // FIXME
    return ActionState::IDLE;
  }
  return ActionState::IDLE;
}

std::ostream &operator<<(std::ostream &Out, const PhysState &PS) {
  Out << "PhysState{Thirst=" << PS.Thirst << ", Hunger=" << PS.Hunger
      << ", Fatigue=" << PS.Fatigue << "}";
  return Out;
}

NeedKind PhysState::getBiggestNeed() const {
  NeedKind Need = NeedKind::NONE;
  if (Fatigue < 1000) {
    Need = NeedKind::SLEEP;
  }
  if (Hunger < 1000) {
    Need = NeedKind::FOOD;
  }
  if (Thirst < 1000) {
    Need = NeedKind::DRINK;
  }
  return Need;
}

void PhysState::update() {
  Thirst -= 5;
  Hunger -= 2;
  Fatigue -= 1;
  // FIXME check > 0
}

} // namespace rogue