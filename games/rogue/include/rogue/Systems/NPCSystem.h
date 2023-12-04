#ifndef ROGUE_SYSTEMS_NPC_SYSTEM_H
#define ROGUE_SYSTEMS_NPC_SYSTEM_H

#include <entt/entt.hpp>
#include <rogue/Components/AI.h>
#include <rogue/Systems/System.h>
#include <ymir/Types.hpp>

namespace rogue {
class Level;
struct PositionComp;
} // namespace rogue

namespace rogue {

class NPCSystem : public System {
public:
  static void decideAction(const PhysState &PS, ReasoningStateComp &RSC);
  static void handleAction(Level &L, entt::entity Entity, PhysState &PS,
                           const PositionComp &PC, ReasoningStateComp &RSC);

  static std::optional<ymir::Point2d<int>>
  searchObject(Level &L, ymir::Point2d<int> Pos, Tile T,
               std::function<void(ymir::Point2d<int>)> FoundCallback);

public:
  explicit NPCSystem(Level &L);
  void update(UpdateType Type) override;

private:
  Level &L;
};

} // namespace rogue

#endif // #ifndef ROGUE_SYSTEMS_NPC_SYSTEM_H