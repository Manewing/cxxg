#ifndef ROGUE_COMPONENTS_AI_H
#define ROGUE_COMPONENTS_AI_H

#include <iosfwd>
#include <optional>
#include <random>
#include <rogue/EffectInfo.h>
#include <rogue/Tile.h>
#include <ymir/Types.hpp>

namespace rogue {
class ItemEffect;
}

namespace rogue {

enum class WanderAIState { Idle = 0, Wander = 1 };
const char *getWanderAIStateStr(WanderAIState State);

struct WanderAIComp {
  WanderAIState State = WanderAIState::Idle;
  unsigned IdleDelay = 10;
  unsigned WanderDelay = 40;

  /// The amount of ticks left in the current state
  unsigned StateDelayLeft = 10;
};

enum class SearchAIState { Idle = 0, Search = 1, Chase = 2 };
const char *getSearchAIStateStr(SearchAIState State);

struct SearchAIComp {
  SearchAIState State = SearchAIState::Idle;

  /// Last position a target was seen at
  std::optional<ymir::Point2d<int>> LastTargetPos;

  /// The amount of ticks to remember the last seen target position, when
  /// the target was lost
  unsigned SearchDuration = 40;
  unsigned SearchDurationLeft = 0;
};

struct AttackAIComp {};

struct EffectExecutorComp {
  struct Info {
    std::shared_ptr<ItemEffect> Effect;
    std::uniform_int_distribution<unsigned> DelayDist;
  };

  std::vector<Info> Effects;
  std::size_t NextEffect = 0;
  unsigned DelayLeft = 0;
};

enum class NeedKind {
  // Model needs like Maslow
  NONE,
  DRINK,
  FOOD,
  SLEEP,
  SHELTER,
  CLOTHING
};
const char *getNeedKindStr(NeedKind Need);
std::ostream &operator<<(std::ostream &Out, NeedKind Need);

enum class ActionState {
  //
  IDLE,
  WANDER,
  SEARCH_DRINK,
  SEARCH_FOOD,
  SLEEP
};
const char *getActionStateStr(ActionState AS);
std::ostream &operator<<(std::ostream &Out, ActionState AS);

ActionState getActionFromNeed(NeedKind Need);

struct PhysState {
  unsigned Thirst = 1000;
  unsigned Hunger = 1000;
  unsigned Fatigue = 1000;

  NeedKind getBiggestNeed() const;
  void update();
};
std::ostream &operator<<(std::ostream &Out, const PhysState &PS);

struct ReasoningStateComp {
  ActionState State = ActionState::IDLE;
  int UpdateCooldown = 5;
};

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_AI_H