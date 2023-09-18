#ifndef ROGUE_SYSTEMS_WANDER_AI_SYSTEM_H
#define ROGUE_SYSTEMS_WANDER_AI_SYSTEM_H

#include <entt/entt.hpp>
#include <rogue/Components/AI.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>
#include <rogue/EventHub.h>
#include <rogue/Systems/System.h>
#include <ymir/Types.hpp>

namespace rogue {
class Level;
}

namespace rogue {

// FIXME for neutral/passive needs to:
//  - flee from incoming attacks
class WanderAISystem : public System {
public:
  explicit WanderAISystem(Level &L, entt::registry &Reg) : System(Reg), L(L) {}
  bool needsTick() const final { return true; }
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

} // namespace rogue

#endif // #ifndef ROGUE_SYSTEMS_AGILITY_SYSTEM_H