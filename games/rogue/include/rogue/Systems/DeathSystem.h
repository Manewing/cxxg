#ifndef ROGUE_SYSTEMS_DEATH_SYSTEM_H
#define ROGUE_SYSTEMS_DEATH_SYSTEM_H

#include <rogue/Systems/System.h>

namespace rogue {

class DeathSystem : public System {
public:
  using System::System;
  void update(UpdateType Type) override;
};

} // namespace rogue

#endif // #ifndef ROGUE_SYSTEMS_DEATH_SYSTEM_H