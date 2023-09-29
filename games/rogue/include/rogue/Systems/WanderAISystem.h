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
struct PositionComp;
struct WanderAIComp;
struct AgilityComp;
struct LineOfSightComp;
struct FactionComp;
} // namespace rogue

namespace rogue {

// FIXME for neutral/passive needs to:
//  - flee from incoming attacks
class WanderAISystem : public System {
public:
  explicit WanderAISystem(Level &L);
  void update(UpdateType Type) override;

private:
  void updateEntity(entt::entity Entity, PositionComp &Pos, WanderAIComp &AI,
                    AgilityComp &Ag);

  std::tuple<entt::entity, const LineOfSightComp *, const FactionComp *>
  checkForTarget(entt::entity Entity, const ymir::Point2d<int> &AtPos);

  ymir::Point2d<int> wander(const ymir::Point2d<int> AtPos);

  std::optional<ymir::Point2d<int>> chaseTarget(entt::entity TargetEt,
                                                const ymir::Point2d<int> AtPos,
                                                const LineOfSightComp &LOS);

private:
  Level &L;
};

} // namespace rogue

#endif // #ifndef ROGUE_SYSTEMS_AGILITY_SYSTEM_H