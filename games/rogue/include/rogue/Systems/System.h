#ifndef ROGUE_SYSTEMS_SYSTEM__H
#define ROGUE_SYSTEMS_SYSTEM__H

#include <entt/entt.hpp>
#include <rogue/EventHub.h>

namespace rogue {

class System : public EventHubConnector {
public:
  enum class UpdateType {
    Tick,
    NoTick
  };

public:
  explicit System(entt::registry &Reg) : Reg(Reg) {}
  virtual ~System() = default;

  /// Run system to update the registry
  virtual void update(UpdateType Type) = 0;

protected:
  entt::registry &Reg;
};

} // namespace rogue

#endif // #ifndef ROGUE_SYSTEMS_SYSTEM_H