#ifndef ROGUE_SYSTEMS_ATTACK_AI_SYSTEM_H
#define ROGUE_SYSTEMS_ATTACK_AI_SYSTEM_H

#include "Systems/System.h"

class AttackAISystem : public System {
public:
  using System::System;
  void update() override;
};

#endif // #ifndef ROGUE_SYSTEMS_ATTACK_AI_SYSTEM_H