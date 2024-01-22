#include <random>
#include <rogue/Components/Entity.h>
#include <rogue/Components/Player.h>
#include <rogue/Components/Transform.h>
#include <rogue/Context.h>
#include <rogue/CraftingDatabase.h>
#include <rogue/EntityDatabase.h>
#include <rogue/Event.h>
#include <rogue/EventHub.h>
#include <rogue/ItemEffectImpl.h>
#include <rogue/Level.h>
#include <rogue/Systems/CombatSystem.h>
#include <sstream>

namespace rogue {

static std::random_device RandomEngine;

std::shared_ptr<ItemEffect> SetMeleeCompEffect::clone() const {
  return std::make_shared<SetMeleeCompEffect>(*this);
}

std::string SetMeleeCompEffect::getName() const { return "Melee Attack"; }

namespace {

std::string getDmgDescription(StatValue Phys, StatValue Magic, StatValue APCost,
                              StatValue ManaCost, const std::string &Type) {
  if (Phys == 0 && Magic == 0) {
    return "no dmg.";
  }

  std::stringstream SS;
  SS << Type << " ";
  const char *Pred = "";
  if (Phys > 0) {
    SS << Pred << Phys << " phys.";
    Pred = " ";
  }
  if (Magic > 0) {
    SS << Pred << Magic << " magic";
    Pred = " ";
  }
  SS << " dmg.";
  if (APCost > 0) {
    SS << " " << APCost << "AP";
  }
  if (ManaCost > 0) {
    SS << " " << ManaCost << "MP";
  }
  return SS.str();
}

} // namespace

std::string SetMeleeCompEffect::getDescription() const {
  return getDmgDescription(Comp.PhysDamage, Comp.MagicDamage, Comp.APCost,
                           Comp.ManaCost, "melee");
}

std::shared_ptr<ItemEffect> SetRangedCompEffect::clone() const {
  return std::make_shared<SetRangedCompEffect>(*this);
}

std::string SetRangedCompEffect::getName() const { return "Ranged Attack"; }

std::string SetRangedCompEffect::getDescription() const {
  return getDmgDescription(Comp.PhysDamage, Comp.MagicDamage, Comp.APCost,
                           Comp.ManaCost, "ranged");
}

std::shared_ptr<ItemEffect> RemovePoisonEffect::clone() const {
  return std::make_shared<RemovePoisonEffect>(*this);
}

std::string RemovePoisonEffect::getName() const {
  return "Remove poison effect";
}

std::string RemovePoisonEffect::getDescription() const {
  return "Removes poison effect";
}

std::shared_ptr<ItemEffect> RemovePoisonDebuffEffect::clone() const {
  return std::make_shared<RemovePoisonDebuffEffect>(*this);
}

std::string RemovePoisonDebuffEffect::getName() const {
  return "Remove poison debuff";
}

std::string RemovePoisonDebuffEffect::getDescription() const {
  return "Removes poison debuff";
}

ManaItemEffect::ManaItemEffect(StatValue Amount) : Amount(Amount) {}

std::shared_ptr<ItemEffect> ManaItemEffect::clone() const {
  return std::make_shared<ManaItemEffect>(*this);
}

std::string ManaItemEffect::getName() const { return "Mana"; }

std::string ManaItemEffect::getDescription() const {
  std::stringstream SS;
  SS << "Restores " << Amount << " mana.";
  return SS.str();
}

bool ManaItemEffect::canApplyTo(const entt::entity &, const entt::entity &DstEt,
                                entt::registry &Reg) const {
  return Reg.all_of<ManaComp>(DstEt);
}

void ManaItemEffect::applyTo(const entt::entity &, const entt::entity &DstEt,
                             entt::registry &Reg) const {
  auto &MC = Reg.get<ManaComp>(DstEt);
  MC.restore(Amount);
}

std::shared_ptr<ItemEffect> SweepingStrikeEffect::clone() const {
  return std::make_shared<SweepingStrikeEffect>(*this);
}

std::string SweepingStrikeEffect::getName() const { return "Sweeping Strike"; }

std::string SweepingStrikeEffect::getDescription() const {
  return "Hits all surrounding enemies as melee damage.";
}

bool SweepingStrikeEffect::canApplyTo(const entt::entity &SrcEt,
                                      const entt::entity &,
                                      entt::registry &Reg) const {
  return Reg.all_of<PositionComp, MeleeAttackComp>(SrcEt);
}

void SweepingStrikeEffect::applyTo(const entt::entity &SrcEt,
                                   const entt::entity &,
                                   entt::registry &Reg) const {
  auto &PC = Reg.get<PositionComp>(SrcEt);

  // Melee is always possible, used default values for damage if not set
  const MeleeAttackComp DefaultMA = {/*PhysDamage =*/1, /*MagicDamage = */ 0,
                                     /*APCost = */ 10};
  MeleeAttackComp MA = DefaultMA;
  auto *AMA = Reg.try_get<MeleeAttackComp>(SrcEt);
  if (AMA) {
    MA = *AMA;
  }

  auto EffMA = MA.getEffective(Reg.try_get<StatsComp>(SrcEt));
  DamageComp DC;
  DC.Source = SrcEt;
  DC.MagicDamage = EffMA.MagicDamage;
  DC.PhysDamage = EffMA.PhysDamage;

  static constexpr auto TempDamageTile =
      Tile{{'*', cxxg::types::RgbColor{175, 175, 175}}};
  for (const auto &Dir : ymir::EightTileDirections<int>::get()) {
    createTempDamage(Reg, DC, PC.Pos + Dir, TempDamageTile);
  }

  // Make sure effect will be rendered
  Reg.ctx().get<GameContext>().EvHub.publish(EffectDelayEvent{});
}

std::shared_ptr<ItemEffect> SmiteEffect::clone() const {
  return std::make_shared<SmiteEffect>(*this);
}

std::string SmiteEffect::getName() const { return "Smite"; }

std::string SmiteEffect::getDescription() const {
  return "Smite a single target with 250% melee damage.";
}

bool SmiteEffect::canApplyTo(const entt::entity &SrcEt,
                             const entt::entity &DstEt,
                             entt::registry &Reg) const {
  return Reg.all_of<MeleeAttackComp>(SrcEt) && Reg.all_of<HealthComp>(DstEt);
}

void SmiteEffect::applyTo(const entt::entity &SrcEt, const entt::entity &DstEt,
                          entt::registry &Reg) const {
  EventHubConnector EHC;
  EHC.setEventHub(&Reg.ctx().get<GameContext>().EvHub);
  CombatSystem::handleMeleeAttack(Reg, SrcEt, DstEt, EHC, 2.5);
}

StompEffect::StompEffect(unsigned Radius, StatValue Damage, Tile EffectTile,
                         double DecreasePercent)
    : Radius(Radius), Damage(Damage), EffectTile(EffectTile),
      DecreasePercent(DecreasePercent) {}

std::shared_ptr<ItemEffect> StompEffect::clone() const {
  return std::make_shared<StompEffect>(*this);
}

std::string StompEffect::getName() const { return "Stomp"; }

std::string StompEffect::getDescription() const {
  std::stringstream SS;
  SS << "Stomps all enemies in a " << static_cast<unsigned>(Radius)
     << " tile radius for " << static_cast<unsigned>(Damage)
     << " phys. damage.";
  if (DecreasePercent > 0) {
    SS << " Damage decreases by " << static_cast<unsigned>(DecreasePercent)
       << "% per tile.";
  }
  return SS.str();
}

bool StompEffect::canApplyTo(const entt::entity &SrcEt, const entt::entity &,
                             entt::registry &Reg) const {
  return Reg.all_of<PositionComp>(SrcEt);
}

namespace {

template <typename HandlerFunc>
void doForEachTileInCircleAt(ymir::Point2d<int> AtPos, int Radius,
                             const HandlerFunc &Handler) {
  // Visit all tiles in radius exactly once
  int X = -Radius, Y = 0, Error = 2 - 2 * Radius; /* II. Quadrant */
  do {
    Handler(AtPos + ymir::Point2d<int>{-X, +Y}); /*   I. Quadrant */
    Handler(AtPos + ymir::Point2d<int>{-Y, -X}); /*  II. Quadrant */
    Handler(AtPos + ymir::Point2d<int>{+X, -Y}); /* III. Quadrant */
    Handler(AtPos + ymir::Point2d<int>{+Y, +X}); /*  IV. Quadrant */
    Radius = Error;
    if (Radius <= Y)
      Error += ++Y * 2 + 1; /* e_xy+e_y < 0 */
    if (Radius > X || Error > Y)
      Error += ++X * 2 + 1; /* e_xy+e_x > 0 or no 2nd y-step */
  } while (X < 0);
}

} // namespace

void StompEffect::applyTo(const entt::entity &SrcEt, const entt::entity &,
                          entt::registry &Reg) const {
  auto &PC = Reg.get<PositionComp>(SrcEt);

  DamageComp DC;
  DC.Source = SrcEt;
  DC.PhysDamage = Damage;

  // Special case for radius 1
  if (Radius > 1) {
    for (const auto &Dir : ymir::EightTileDirections<int>::get()) {
      createTempDamage(Reg, DC, PC.Pos + Dir, EffectTile);
    }
  }

  // Visit all tiles in radius exactly once
  static const auto DecreaseFactor = (1 - DecreasePercent / 100.0);
  for (unsigned X = 2; X < Radius; ++X) {
    // Phsyical damage decreases by given percentage per tile
    DC.PhysDamage *= DecreaseFactor;
    doForEachTileInCircleAt(PC.Pos, X, [this, &Reg, &DC](ymir::Point2d<int> P) {
      createTempDamage(Reg, DC, P, EffectTile);
    });
  }

  // Make sure effect will be rendered
  Reg.ctx().get<GameContext>().EvHub.publish(EffectDelayEvent{});
}

std::shared_ptr<ItemEffect> LearnRecipeEffect::clone() const {
  return std::make_shared<LearnRecipeEffect>(*this);
}

std::string LearnRecipeEffect::getName() const { return "Learn Recipe"; }

std::string LearnRecipeEffect::getDescription() const {
  return "Learns a random recipe";
}

bool LearnRecipeEffect::canApplyTo(const entt::entity &,
                                   const entt::entity &DstEt,
                                   entt::registry &Reg) const {
  return Reg.all_of<PlayerComp>(DstEt);
}

void LearnRecipeEffect::applyTo(const entt::entity &, const entt::entity &DstEt,
                                entt::registry &Reg) const {
  auto *PC = Reg.try_get<PlayerComp>(DstEt);
  if (!PC) {
    return;
  }
  auto &CraftingDb = Reg.ctx().get<GameContext>().CraftingDb;
  auto &EvHub = Reg.ctx().get<GameContext>().EvHub;

  auto &Recipes = CraftingDb.getRecipes();
  if (Recipes.empty()) {
    return;
  }

  std::vector<CraftingRecipeId> AvailableRecipes;
  for (const auto &[RecipeId, Recipe] : Recipes) {
    if (!PC->KnownRecipes.count(RecipeId)) {
      AvailableRecipes.push_back(RecipeId);
    }
  }

  if (AvailableRecipes.empty()) {
    return;
  }

  auto Idx = std::rand() % AvailableRecipes.size();
  const auto &RecipeId = AvailableRecipes[Idx];

  PC->KnownRecipes.insert(RecipeId);
  EvHub.publish(PlayerInfoMessageEvent()
                << "You learned: " << Recipes.at(RecipeId).getName());
}

SpawnEntityEffect::SpawnEntityEffect(std::string EntityName, double Chance)
    : EntityName(std::move(EntityName)), Chance(Chance) {}

std::shared_ptr<ItemEffect> SpawnEntityEffect::clone() const {
  return std::make_shared<SpawnEntityEffect>(*this);
}

std::string SpawnEntityEffect::getName() const { return "Spawn " + EntityName; }

std::string SpawnEntityEffect::getDescription() const {
  std::stringstream SS;
  SS << "Spawns " << EntityName;
  if (Chance != 0) {
    SS << " with " << Chance * 100 << "% chance.";
  }
  return SS.str();
}

bool SpawnEntityEffect::canApplyTo(const entt::entity &,
                                   const entt::entity &DstEt,
                                   entt::registry &Reg) const {
  return Reg.all_of<PositionComp>(DstEt);
}

void SpawnEntityEffect::applyTo(const entt::entity &, const entt::entity &DstEt,
                                entt::registry &Reg) const {
  auto *Lvl = Reg.ctx().get<Level *>();
  assert(Lvl && "No level set in registry");

  if (Chance != 0) {
    std::uniform_real_distribution<double> Dist(0, 1);
    if (Dist(RandomEngine) > Chance) {
      return;
    }
  }

  auto &Pos = Reg.get<PositionComp>(DstEt).Pos;
  auto &EvHub = Reg.ctx().get<GameContext>().EvHub;

  const auto &EntityDb = Reg.ctx().get<GameContext>().EntityDb;
  EntityFactory EF(Reg, EntityDb);

  auto EtTmplId = EntityDb.getEntityTemplateId(EntityName);
  auto NewEntity = EF.createEntity(EtTmplId);

  if (auto *PC = Reg.try_get<PositionComp>(NewEntity)) {
    auto NewPosOrNone = Lvl->getNonBodyBlockedPosNextTo(Pos);
    if (!NewPosOrNone) {
      EvHub.publish(PlayerInfoMessageEvent()
                    << "Could not spawn " << EntityName << " at " << Pos
                    << " because there is no free space.");
      return;
    }
    PC->Pos = *NewPosOrNone;
  }

  EvHub.publish(SpawnEntityEvent{{}, NewEntity, &Reg});
}

} // namespace rogue