#ifndef ROGUE_SYSTEMS_PLAYER_SYSTEM_H
#define ROGUE_SYSTEMS_PLAYER_SYSTEM_H

#include <rogue/Systems/System.h>

namespace rogue {

class Level;

class PlayerSystem : public System {
public:
  explicit PlayerSystem(Level &L);
  void update(UpdateType Type) override;

private:
  Level &L;
};

} // namespace rogue

#endif // #ifndef ROGUE_SYSTEMS_PLAYER_SYSTEM_H