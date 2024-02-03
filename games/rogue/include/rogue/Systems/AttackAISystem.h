#ifndef ROGUE_SYSTEMS_ATTACK_AI_SYSTEM_H
#define ROGUE_SYSTEMS_ATTACK_AI_SYSTEM_H

#include <rogue/Systems/System.h>

namespace rogue {
class Level;
}

namespace rogue {

class AttackAISystem : public System {
public:
  explicit AttackAISystem(Level &L);
  void update(UpdateType Type) override;

private:
  Level &L;
};

} // namespace rogue

#endif // #ifndef ROGUE_SYSTEMS_ATTACK_AI_SYSTEM_H