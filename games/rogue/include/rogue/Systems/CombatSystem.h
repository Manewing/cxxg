#ifndef ROGUE_SYSTEMS_COMBAT_SYSTEM_H
#define ROGUE_SYSTEMS_COMBAT_SYSTEM_H

#include <rogue/Systems/System.h>

namespace rogue {

class CombatSystem : public System {
public:
  using System::System;
  void update(UpdateType Type) override;
};

} // namespace rogue

#endif // ROGUE_SYSTEMS_COMBAT_SYSTEM_H