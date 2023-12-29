#include <rogue/Components/Player.h>
#include <rogue/Components/Transform.h>
#include <rogue/Context.h>
#include <rogue/CraftingDatabase.h>
#include <rogue/Event.h>
#include <rogue/EventHub.h>
#include <rogue/ItemEffectImpl.h>
#include <sstream>

namespace rogue {

std::shared_ptr<ItemEffect> SetMeleeCompEffect::clone() const {
  return std::make_shared<SetMeleeCompEffect>(*this);
}

std::string SetMeleeCompEffect::getName() const { return "Melee Attack"; }

namespace {

std::string getDmgDescription(StatValue Phys, StatValue Magic,
                              const std::string &Type) {
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
  return SS.str();
}

} // namespace

std::string SetMeleeCompEffect::getDescription() const {
  return getDmgDescription(Comp.PhysDamage, Comp.MagicDamage, "melee");
}

std::shared_ptr<ItemEffect> SetRangedCompEffect::clone() const {
  return std::make_shared<SetRangedCompEffect>(*this);
}

std::string SetRangedCompEffect::getName() const { return "Ranged Attack"; }

std::string SetRangedCompEffect::getDescription() const {
  return getDmgDescription(Comp.PhysDamage, Comp.MagicDamage, "ranged");
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
  return "Hits all surrounding enemies.";
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
  const MeleeAttackComp DefaultMA = {
      .PhysDamage = 1, .MagicDamage = 0, .APCost = 10};
  MeleeAttackComp MA = DefaultMA;
  auto *AMA = Reg.try_get<MeleeAttackComp>(SrcEt);
  if (AMA) {
    MA = *AMA;
  }

  DamageComp DC;
  DC.Source = SrcEt;
  if (auto *SC = Reg.try_get<StatsComp>(SrcEt)) {
    auto SP = SC->effective();
    DC.PhysDamage = MA.getPhysEffectiveDamage(&SP);
    DC.MagicDamage = MA.getMagicEffectiveDamage(&SP);
  } else {
    DC.PhysDamage = MA.getPhysEffectiveDamage();
    DC.MagicDamage = MA.getMagicEffectiveDamage();
  }

  for (const auto &Dir : ymir::EightTileDirections<int>::get()) {
    createTempDamage(Reg, DC, PC.Pos + Dir);
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

} // namespace rogue