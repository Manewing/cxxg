#ifndef ROGUE_MOVEMENT_SYSTEM_H
#define ROGUE_MOVEMENT_SYSTEM_H

#include <rogue/Systems/System.h>

namespace rogue {
class Level;
}

namespace rogue {

class MovementSystem : public System {
public:
  explicit MovementSystem(Level &L);
  void update(UpdateType Type) override;

private:
  Level &L;
};

} // namespace rogue

#endif // #ifndef ROGUE_MOVEMENT_SYSTEM_H