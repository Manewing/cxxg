#include <rogue/Components/Player.h>
#include <rogue/Event.h>

namespace rogue {

namespace {

bool isValidAndPlayer(const entt::registry *Reg, entt::entity Entity) {
  if (!Reg || Entity == entt::null) {
    return false;
  }
  return Reg->try_get<PlayerComp>(Entity) != nullptr;
}

} // namespace

bool EntityAttackEvent::isPlayerAffected() const {
  return isValidAndPlayer(Registry, Attacker) ||
         isValidAndPlayer(Registry, Target);
}

bool EntityDiedEvent::isPlayerAffected() const {
  return isValidAndPlayer(Registry, Entity);
}

bool LootEvent::isPlayerAffected() const {
  return isValidAndPlayer(Registry, Entity);
}

bool CraftEvent::isPlayerAffected() const {
  return isValidAndPlayer(Registry, Entity);
}

bool BuffAppliedEvent::isPlayerAffected() const {
  return isValidAndPlayer(Registry, SrcEt) ||
         isValidAndPlayer(Registry, TargetEt);
}

bool BuffApplyEffectEvent::isPlayerAffected() const {
  return isValidAndPlayer(Registry, Entity);
}

bool BuffExpiredEvent::isPlayerAffected() const {
  return isValidAndPlayer(Registry, Entity);
}

} // namespace rogue