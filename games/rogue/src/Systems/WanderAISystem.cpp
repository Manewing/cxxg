#include <rogue/Components/AI.h>
#include <rogue/Components/Transform.h>
#include <rogue/Level.h>
#include <rogue/Systems/WanderAISystem.h>
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
  // TODO flee from attacker
  // auto *CAC = Reg.try_get<CombatTargetComp>(Entity);
  // if (CAC && Reg.valid(CAC->Attacker)) {
  //  AI.State = WanderAIState::Flee;
  //}

  switch (AI.State) {
  case WanderAIState::Idle: {
    if (AI.StateDelayLeft-- == 0) {
      AI.State = WanderAIState::Wander;
      AI.StateDelayLeft = AI.WanderDelay;
    }
  } break;
  case WanderAIState::Wander: {
    if (AI.StateDelayLeft-- == 0) {
      AI.State = WanderAIState::Idle;
      AI.StateDelayLeft = AI.IdleDelay;
    }
    auto NextPos = findRandomNonBlockedPosNextTo(PC);
    MovementComp MC;
    MC.Dir = ymir::Dir2d::fromMaxComponent(NextPos - PC.Pos);
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

ymir::Point2d<int>
WanderAISystem::findRandomNonBlockedPosNextTo(ymir::Point2d<int> AtPos) const {
  auto AllNextPos = L.getAllNonBodyBlockedPosNextTo(AtPos);
  auto It =
      ymir::randomIterator(AllNextPos.begin(), AllNextPos.end(), RandomEngine);
  if (AllNextPos.empty() || It == AllNextPos.end()) {
    return AtPos;
  }
  return *It;
}

} // namespace rogue