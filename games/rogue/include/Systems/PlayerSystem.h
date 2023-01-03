#ifndef ROGUE_SYSTEMS_PLAYER_SYSTEM_H
#define ROGUE_SYSTEMS_PLAYER_SYSTEM_H

#include "Systems/System.h"

class Level;

class PlayerSystem : public System {
public:
  explicit PlayerSystem(Level &L);
  void update() override;

private:
  Level &L;
};

#endif // #ifndef ROGUE_SYSTEMS_PLAYER_SYSTEM_H