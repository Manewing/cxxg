#ifndef ROGUE_SYSTEMS_WANDER_AI_SYSTEM_H
#define ROGUE_SYSTEMS_WANDER_AI_SYSTEM_H

#include <entt/entt.hpp>
#include <rogue/Systems/System.h>
#include <ymir/Types.hpp>

namespace rogue {
class Level;
struct PositionComp;
struct WanderAIComp;
} // namespace rogue

namespace rogue {

// FIXME for neutral/passive needs to:
//  - flee from incoming attacks
class WanderAISystem : public System {
public:
  explicit WanderAISystem(Level &L);
  void update(UpdateType Type) override;

private:
  void updateEntity(entt::entity Entity, PositionComp &Pos, WanderAIComp &AI);

  /// Searches the level for a non-blocked position next to \p AtPos
  ymir::Point2d<int>
  findRandomNonBlockedPosNextTo(ymir::Point2d<int> AtPos) const;

private:
  Level &L;
};

} // namespace rogue

#endif // #ifndef ROGUE_SYSTEMS_WANDER_AI_SYSTEM_H