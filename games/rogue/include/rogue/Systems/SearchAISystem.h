#ifndef ROGUE_SYSTEMS_SEARCH_AI_SYSTEM_H
#define ROGUE_SYSTEMS_SEARCH_AI_SYSTEM_H

#include <entt/entt.hpp>
#include <rogue/Systems/System.h>
#include <ymir/Types.hpp>

namespace rogue {
class Level;
struct PositionComp;
struct SearchAIComp;
struct LineOfSightComp;
struct FactionComp;
} // namespace rogue

namespace rogue {

class SearchAISystem : public System {
public:
  explicit SearchAISystem(Level &L);
  void update(UpdateType Type) override;

private:
  void updateEntity(entt::entity Entity, PositionComp &Pos, SearchAIComp &AI);

  std::tuple<entt::entity, const LineOfSightComp *, const FactionComp *>
  checkForTarget(entt::entity Entity, PositionComp &PC, SearchAIComp &AI);

  std::tuple<entt::entity, const LineOfSightComp *, const FactionComp *>
  findTarget(entt::entity Entity, const ymir::Point2d<int> &AtPos);

  std::optional<ymir::Point2d<int>>
  findPathToPoint(const ymir::Point2d<int> ToPos,
                  const ymir::Point2d<int> FutureToPos,
                  const ymir::Point2d<int> AtPos, const unsigned LOSRange);

  std::optional<ymir::Point2d<int>> chaseTarget(entt::entity TargetEt,
                                                const ymir::Point2d<int> AtPos,
                                                const LineOfSightComp &LOS);

private:
  Level &L;
};

} // namespace rogue

#endif // #ifndef ROGUE_SYSTEMS_SEARCH_AI_SYSTEM_H