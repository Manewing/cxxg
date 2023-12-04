#ifndef ROGUE_COMPONENTS_PLAYER_H
#define ROGUE_COMPONENTS_PLAYER_H

#include <entt/entt.hpp>
#include <functional>
#include <optional>
#include <string>
#include <ymir/Types.hpp>

namespace rogue {
class EventHubConnector;
} // namespace rogue

namespace rogue {

struct Interaction {
  std::string Msg;
  std::function<void(EventHubConnector &, entt::entity, entt::registry &)>
      Execute = [](auto &, auto, auto &) {};
};

// FIXME allow having multiple action being caused by one interaction
struct InteractableComp {
  Interaction Action;
};

struct PlayerComp {
public:
  static entt::entity createPlayer(entt::registry &Reg, const std::string &Name,
                                   ymir::Point2d<int> Pos = {});
  static entt::entity copyPlayer(entt::registry &RegFrom,
                                 entt::registry &RegTo);
  static entt::entity movePlayer(entt::registry &RegFrom,
                                 entt::registry &RegTo);
  static void removePlayer(entt::registry &Reg);

public:
  std::optional<Interaction> CurrentInteraction;
  ymir::Dir2d MoveDir = ymir::Dir2d::NONE;
  bool IsReady = false;
};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_PLAYER_H