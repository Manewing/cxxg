#ifndef ROGUE_SYSTEMS_AGILITY_SYSTEM_H
#define ROGUE_SYSTEMS_AGILITY_SYSTEM_H

#include "Systems/System.h"

// FIXME join into RegenSystem?
class AgilitySystem : public System {
public:
  using System::System;
  void update() override;
};

#endif // #ifndef ROGUE_SYSTEMS_AGILITY_SYSTEM_H