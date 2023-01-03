#ifndef ROGUE_SYSTEMS_DEATH_SYSTEM_H
#define ROGUE_SYSTEMS_DEATH_SYSTEM_H

#include "Systems/System.h"

struct EntityDiedEvent : public Event {
  entt::entity Entity;
  bool IsPlayer = false;
};

class DeathSystem : public System {
public:
  using System::System;
  void update() override;
};

#endif // #ifndef ROGUE_SYSTEMS_DEATH_SYSTEM_H