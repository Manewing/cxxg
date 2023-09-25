#include <rogue/Components/Buffs.h>
#include <rogue/Components/Stats.h>
#include <rogue/Systems/StatsSystem.h>

namespace rogue {

namespace {

static constexpr float HealthPerVit = 9.1f;
static constexpr float ManaPerInt = 9.1f;
static constexpr float AgilityPerDex = 1.0f;

void resetStats(entt::registry &Reg) {
  Reg.view<StatsComp>().each([](auto &S) { S.reset(); });
}

void updateStatsTimedBuffComp(entt::entity Entity, entt::registry &Reg,
                              StatsComp &S, StatsTimedBuffComp &STB) {
  if (STB.tick()) {
    Reg.erase<StatsTimedBuffComp>(Entity);
    return;
  }
  S.add(STB.Bonus);
}

void updateStatsBuffPerHitComp(entt::entity Entity, entt::registry &Reg,
                               StatsBuffPerHitComp &SBPH) {

  auto *SC = Reg.try_get<StatsBuffComp>(Entity);
  if (SC && SBPH.Applied) {
    if (SC->remove(SBPH.getEffectiveBuff())) {
      Reg.erase<StatsBuffComp>(Entity);
      SC = nullptr;
    }
    SBPH.Applied = false; 
  }

  if (SBPH.tick()) {
    return;
  }

  if (SC) {
    SC->add(SBPH.getEffectiveBuff());
  } else {
    Reg.emplace<StatsBuffComp>(Entity, SBPH.getEffectiveBuff());
  }

  SBPH.Applied = true;
}

void applyTimedStatsAndBuffs(entt::registry &Reg) {
  // Apply timed stats buff
  Reg.view<StatsComp, StatsTimedBuffComp>().each(
      [&Reg](auto Entity, auto &S, auto &STB) {
        updateStatsTimedBuffComp(Entity, Reg, S, STB);
      });

  // Update stats buff per hit comp
  Reg.view<StatsBuffPerHitComp>().each([&Reg](auto Entity, auto &SBPH) {
    updateStatsBuffPerHitComp(Entity, Reg, SBPH);
  });
}

void applyStaticStatBuffs(entt::registry &Reg) {
  // Apply static stats buff
  Reg.view<StatsComp, const StatsBuffComp>().each(
      [](auto &S, auto const &SB) { S.add(SB.Bonus); });
}

void applyStatEffects(entt::registry &Reg) {
  // Health
  // If the entity has stats they override the maximum values defined
  Reg.view<const StatsComp, HealthComp>().each(
      [](const auto &St, auto &Health) {
        Health.MaxValue = St.effective().Vit * HealthPerVit;
        if (Health.Value > Health.MaxValue) {
          Health.Value = Health.MaxValue;
        }
      });

  // Mana
  // If the entity has stats they override the maximum values defined
  Reg.view<const StatsComp, ManaComp>().each([](const auto &St, auto &Mana) {
    Mana.MaxValue = St.effective().Int * ManaPerInt;
    if (Mana.Value > Mana.MaxValue) {
      Mana.Value = Mana.MaxValue;
    }
  });

  // Agility is determined by Dex
  Reg.view<const StatsComp, AgilityComp>().each([](const auto &St, auto &Ag) {
    Ag.Agility = AgilityPerDex * St.effective().Dex;
  });
}

} // namespace

void StatsSystem::update(UpdateType Type) {
  resetStats(Reg);

  if (Type == UpdateType::Tick) {
    applyTimedStatsAndBuffs(Reg);
  }

  applyStaticStatBuffs(Reg);
  applyStatEffects(Reg);
}

} // namespace rogue
