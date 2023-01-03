#ifndef ROGUE_SYSTEMS_WANDER_AI_SYSTEM_H
#define ROGUE_SYSTEMS_WANDER_AI_SYSTEM_H

#include <entt/entt.hpp>
#include <ymir/Types.hpp>

#include "Components/AI.h"
#include "Components/Stats.h"
#include "Components/Transform.h"
#include "EventHub.h"
#include "Systems/System.h"

class Level;

// FIXME for neutral/passive needs to:
//  - flee from incoming attacks
class WanderAISystem : public System {
public:
  explicit WanderAISystem(Level &L, entt::registry &Reg) : System(Reg), L(L) {}
  void update() override;

private:
  std::optional<entt::entity> checkForTarget(const entt::entity &Entity,
                                             const ymir::Point2d<int> &AtPos);

  ymir::Point2d<int> wander(const ymir::Point2d<int> AtPos);

  ymir::Point2d<int> chaseTarget(const ymir::Point2d<int> AtPos,
                                 const entt::entity &Target);

private:
  Level &L;
};

#endif // #ifndef ROGUE_SYSTEMS_AGILITY_SYSTEM_H