#ifndef ROGUE_EVENT_H
#define ROGUE_EVENT_H

#include <entt/entt.hpp>
#include <optional>
#include <sstream>

namespace rogue {
struct BuffBase;
}

namespace rogue {

// Base class for all events
struct BaseEvent {
  // virtual ~BaseEvent() = default;
};

template <typename DerivedT> struct BaseMessageEvent : public BaseEvent {
  std::stringstream Message;

  template <typename Type> DerivedT &operator<<(const Type &T) {
    Message << T;
    return static_cast<DerivedT &>(*this);
  }
};

struct DebugMessageEvent : public BaseMessageEvent<DebugMessageEvent> {};
struct WarningMessageEvent : public BaseMessageEvent<WarningMessageEvent> {};
struct ErrorMessageEvent : public BaseMessageEvent<ErrorMessageEvent> {};
struct PlayerInfoMessageEvent
    : public BaseMessageEvent<PlayerInfoMessageEvent> {};

struct EntityAttackEvent : public BaseEvent {
  entt::entity Attacker = entt::null;
  entt::entity Target = entt::null;
  entt::registry *Registry = nullptr;

  /// Damage that was dealt to target, nullopt for blocks
  std::optional<unsigned> Damage = std::nullopt;

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

struct DetectTargetEvent : public BaseEvent {
  entt::entity Entity = entt::null;
  entt::entity Target = entt::null;
  entt::registry *Registry = nullptr;
};

struct LostTargetEvent : public BaseEvent {
  entt::entity Entity = entt::null;
  entt::registry *Registry = nullptr;
};

} // namespace rogue

#endif // #ifndef ROGUE_EVENT_H