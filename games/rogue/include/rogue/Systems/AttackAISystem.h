#ifndef ROGUE_SYSTEMS_ATTACK_AI_SYSTEM_H
#define ROGUE_SYSTEMS_ATTACK_AI_SYSTEM_H

#include <rogue/Systems/System.h>

namespace rogue {

class AttackAISystem : public System {
public:
  using System::System;
  bool needsTick() const final { return true; }
  void update() override;
};

} // namespace rogue

#endif // #ifndef ROGUE_SYSTEMS_ATTACK_AI_SYSTEM_H