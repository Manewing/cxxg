#ifndef ROGUE_SYSTEMS_SYSTEM__H
#define ROGUE_SYSTEMS_SYSTEM__H

#include <entt/entt.hpp>
#include <rogue/EventHub.h>

namespace rogue {

class System : public EventHubConnector {
public:
  explicit System(entt::registry &Reg) : Reg(Reg) {}
  virtual ~System() = default;

  /// Wether the system needs a game tick to update
  virtual bool needsTick() const { return false; }

  /// Run system to update the registry
  virtual void update() = 0;

protected:
  entt::registry &Reg;
};

} // namespace rogue

#endif // #ifndef ROGUE_SYSTEMS_SYSTEM_H