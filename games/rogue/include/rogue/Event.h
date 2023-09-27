#ifndef ROGUE_EVENT_H
#define ROGUE_EVENT_H

#include <entt/entt.hpp>
#include <sstream>

namespace rogue {
struct BuffBase;
}

namespace rogue {

// Base class for all events
struct BaseEvent {
  // virtual ~BaseEvent() = default;
};

struct BaseMessageEvent : public BaseEvent {
  std::stringstream Message;

  template <typename Type> BaseMessageEvent &operator<<(const Type &T) {
    Message << T;
    return *this;
  }
};

struct DebugMessageEvent : public BaseMessageEvent {};
struct WarningMessageEvent : public BaseMessageEvent {};
struct ErrorMessageEvent : public BaseMessageEvent {};
struct PlayerInfoMessageEvent : public BaseMessageEvent {};

struct EntityAttackEvent : public BaseEvent {
  entt::entity Attacker = entt::null;
  entt::entity Target = entt::null;
  entt::registry *Registry = nullptr;
  unsigned Damage = 0;
  bool isPlayerAffected() const;
};

struct EntityDiedEvent : public BaseEvent {
  entt::entity Entity = entt::null;
  entt::registry *Registry = nullptr;
  bool isPlayerAffected() const;
};

struct BuffExpiredEvent : public BaseEvent {
  entt::entity Entity = entt::null;
  const BuffBase *Buff = nullptr;
};

struct SwitchLevelEvent : public BaseEvent {
  int Level = 0;
  bool ToEntry = false;
};

struct LootEvent : public BaseEvent {
  entt::entity Entity = entt::null;
  entt::entity LootedEntity = entt::null;
  entt::registry *Registry = nullptr;

  bool isPlayerAffected() const;
};

} // namespace rogue

#endif // #ifndef ROGUE_EVENT_H