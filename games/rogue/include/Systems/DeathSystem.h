#ifndef ROGUE_SYSTEMS_DEATH_SYSTEM_H
#define ROGUE_SYSTEMS_DEATH_SYSTEM_H

#include "EventHub.h"
#include <entt/entt.hpp>

struct EntityDiedEvent : public Event {
  entt::entity Entity;
  bool IsPlayer = false;
};

class DeathSystem : public EventHubConnector {
public:
  explicit DeathSystem(entt::registry &Reg) : Reg(Reg) {}

  void update();

private:
  entt::registry &Reg;
};

#endif // #ifndef ROGUE_SYSTEMS_DEATH_SYSTEM_H