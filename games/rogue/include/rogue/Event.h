#ifndef ROGUE_EVENT_H
#define ROGUE_EVENT_H

#include <entt/entt.hpp>
#include <filesystem>
#include <optional>
#include <sstream>

namespace rogue {
struct BuffBase;
struct DiminishingReturnsValueGenBuff;
} // namespace rogue

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
inline bool operator==(const EntityDiedEvent &Lhs, const EntityDiedEvent &Rhs) {
  return Lhs.Entity == Rhs.Entity && Lhs.Registry == Rhs.Registry;
}

/// Can be published to introduce a delay when rendering for an effect
struct EffectDelayEvent : public BaseEvent {};

struct BuffAppliedEvent : public BaseEvent {
  entt::entity SrcEt = entt::null;
  entt::entity TargetEt = entt::null;
  entt::registry *Registry = nullptr;
  bool IsCombat = false;
  const BuffBase *Buff = nullptr;

  bool isPlayerAffected() const;
};

struct BuffApplyEffectEvent : public BaseEvent {
  entt::entity Entity = entt::null;
  entt::registry *Registry = nullptr;
  bool IsReduce = false;
  const DiminishingReturnsValueGenBuff *Buff = nullptr;

  bool isPlayerAffected() const;
};

struct RestoreHealthEvent : public BaseEvent {
  entt::entity Entity = entt::null;
  entt::registry *Registry = nullptr;
  unsigned Amount = 0;
  std::string RestoreSource;
  bool isPlayerAffected() const;
};

struct BuffExpiredEvent : public BaseEvent {
  entt::entity Entity = entt::null;
  entt::registry *Registry = nullptr;
  const BuffBase *Buff = nullptr;

  bool isPlayerAffected() const;
};

struct SwitchLevelEvent : public BaseEvent {
  int Level = 0;
  bool ToEntry = false;
  entt::entity TriggerEt = entt::null;
  entt::entity SwitchEt = entt::null;
};

struct SwitchGameWorldEvent : public BaseEvent {
  std::string LevelName;
  entt::entity TriggerEt = entt::null;
  entt::entity SwitchEt = entt::null;
};

struct LootEvent : public BaseEvent {
  std::string LootName;
  entt::entity Entity = entt::null;
  entt::entity LootedEntity = entt::null;
  entt::registry *Registry = nullptr;

  bool isPlayerAffected() const;
};

struct CraftEvent : public BaseEvent {
  entt::entity Entity = entt::null;
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