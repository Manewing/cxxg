#ifndef ROGUE_SYSTEMS_WANDER_AI_SYSTEM_H
#define ROGUE_SYSTEMS_WANDER_AI_SYSTEM_H

#include <entt/entt.hpp>
#include <ymir/Map.hpp>
#include <ymir/Types.hpp>

#include "EventHub.h"
#include "Components/AI.h"
#include "Components/Stats.h"
#include "Components/Transform.h"

class Level;

// FIXME for aggressive entities needs to:
//  - chase entity target to damage
// FIXME for neutral/passive needs to:
//  - flee from incoming attacks
class WanderAISystem : public EventHubConnector {
public:
  explicit WanderAISystem(Level &L, entt::registry &Reg) : L(L), Reg(Reg) {}
  void update();

private:
  void updateEntityPosCache();

  std::optional<entt::entity> checkForTarget(const entt::entity &Entity,
                                             const ymir::Point2d<int> &AtPos);

  ymir::Point2d<int> wander(const ymir::Point2d<int> AtPos);

  ymir::Point2d<int> chaseTarget(const ymir::Point2d<int> AtPos,
                                 const entt::entity &Target);

private:
  Level &L;
  entt::registry &Reg;

  // FIXME make this part of the context?
  ymir::Map<entt::entity> EntityPosCache;
};

#endif // #ifndef ROGUE_SYSTEMS_AGILITY_SYSTEM_H