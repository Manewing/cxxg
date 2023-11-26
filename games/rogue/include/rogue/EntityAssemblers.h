#ifndef ROGUE_ENTITY_ASSEMBLERS_H
#define ROGUE_ENTITY_ASSEMBLERS_H

#include <entt/entt.hpp>
#include <ymir/Types.hpp>
#include <rogue/EntityDatabase.h>
#include <rogue/Components/Visual.h>

#include <rogue/Components/AI.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Entity.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/LOS.h>
#include <rogue/Components/Level.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/RaceFaction.h>
#include <rogue/Components/Stats.h>
#include <rogue/Components/Transform.h>


namespace rogue {

template <typename T>
class DefaultConstructEntityAssembler : public EntityAssembler {
public:
  void assemble(entt::registry &Reg, entt::entity Entity) const override {
    Reg.emplace<T>(Entity);
  }
};

class TileCompAssembler : public EntityAssembler {
public:
  void assemble(entt::registry &Reg, entt::entity Entity) const override;

private:
    Tile T;
};

// Keep sorted
using AttackAICompAssembler = DefaultConstructEntityAssembler<AttackAIComp>;
using CollisionCompAssembler = DefaultConstructEntityAssembler<CollisionComp>;
using HealthCompAssembler = DefaultConstructEntityAssembler<HealthComp>;
using PlayerCompAssembler = DefaultConstructEntityAssembler<PlayerComp>;
using PositionCompAssembler = DefaultConstructEntityAssembler<PositionComp>;
using VisibleCompAssembler = DefaultConstructEntityAssembler<VisibleComp>;
using WanderAICompAssembler = DefaultConstructEntityAssembler<WanderAIComp>;

// TODO make configurable
using LineOfSightCompAssembler = DefaultConstructEntityAssembler<LineOfSightComp>;
using AgilityCompAssembler = DefaultConstructEntityAssembler<AgilityComp>;

}

#endif // #ifndef ROGUE_ENTITY_ASSEMBLERS_H