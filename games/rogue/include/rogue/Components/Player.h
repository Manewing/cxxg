#ifndef ROGUE_COMPONENTS_PLAYER_H
#define ROGUE_COMPONENTS_PLAYER_H

#include <entt/entt.hpp>
#include <functional>
#include <optional>
#include <string>
#include <ymir/Types.hpp>

namespace rogue {
class Game;
}

namespace rogue {

struct Interaction {
  std::string Msg;
  std::function<void(Game &, entt::entity, entt::registry &)> Execute =
      [](auto &, auto, auto &) {};
};

struct InteractableComp {
  Interaction Action;
};

struct PlayerComp {
public:
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
};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_PLAYER_H