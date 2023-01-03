#ifndef ROGUE_SYSTEMS_SYSTEM__H
#define ROGUE_SYSTEMS_SYSTEM__H

#include <entt/entt.hpp>
#include "EventHub.h"

class System : public EventHubConnector {
public:
  explicit System(entt::registry &Reg) : Reg(Reg) {}
  virtual ~System() = default;

  /// Run system to update the registry
  virtual void update() = 0;

protected:
  entt::registry &Reg;
};

#endif // #ifndef ROGUE_SYSTEMS_SYSTEM_H