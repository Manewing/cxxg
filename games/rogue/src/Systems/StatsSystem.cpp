#include <rogue/Components/Buffs.h>
#include <rogue/Components/Stats.h>
#include <rogue/Systems/StatsSystem.h>

namespace rogue {

namespace {

static constexpr float HealthPerVit = 9.1f;
static constexpr float HealthRegenPerVit = 0.02f;
static constexpr float ManaPerInt = 9.1f;
static constexpr float ManaRegenPerInt = 0.02f;

// Based on minimum AP cost of 10, see Balancing sheet
static constexpr float AgilityPerDexLogBase = 1.2f;
static constexpr float AgilityPerDexScale = 10.0f;
static constexpr float AgilityPerDexLogOffset = 1.0f;

void resetStats(entt::registry &Reg) {
  Reg.view<StatsComp>().each([](auto &S) { S.reset(); });
}

void updateStatsTimedBuffComp(entt::entity Entity, entt::registry &Reg,
                              StatsComp &S, StatsTimedBuffComp &STB) {
  auto St = STB.tick();
  if (St == TimedBuff::State::Expired) {
    Reg.erase<StatsTimedBuffComp>(Entity);
    return;
  }
  S.add(STB.Bonus);
}

void updateStatsBuffPerHitComp(entt::entity Entity, entt::registry &Reg,
                               StatsBuffPerHitComp &SBPH, bool Tick) {
  if (!SBPH.Stacks) {
    return;
  }

  auto *SC = Reg.try_get<StatsBuffComp>(Entity);
  if (SC && SBPH.AppliedStack) {
    if (SC->remove(SBPH.getEffectiveBuff(*SBPH.AppliedStack))) {
      Reg.erase<StatsBuffComp>(Entity);
      SC = nullptr;
    }
    SBPH.Applied = nullptr;
    SBPH.AppliedStack = std::nullopt;
  }

  if (Tick && SBPH.tick()) {
    SBPH.AppliedStack = std::nullopt;
    return;
  }

  if (SC) {
    SC->add(SBPH.getEffectiveBuff(SBPH.Stacks));
  } else {
    SC =
        &Reg.emplace<StatsBuffComp>(Entity, SBPH.getEffectiveBuff(SBPH.Stacks));
  }
  SBPH.Applied = SC;
  SBPH.AppliedStack = SBPH.Stacks;
}

void applyStatsBuffPerHitComp(entt::registry &Reg, bool Tick) {
  Reg.view<StatsBuffPerHitComp>().each([&Reg, Tick](auto Entity, auto &SBPH) {
    updateStatsBuffPerHitComp(Entity, Reg, SBPH, Tick);
  });
}

void applyTimedStatsAndBuffs(entt::registry &Reg) {
  // Apply timed stats buff
  Reg.view<StatsComp, StatsTimedBuffComp>().each(
      [&Reg](auto Entity, auto &S, auto &STB) {
        updateStatsTimedBuffComp(Entity, Reg, S, STB);
      });
}

void applyStaticStatBuffs(entt::registry &Reg) {
  // Apply static stats buff
  Reg.view<StatsComp, StatsBuffComp>().each(
      [&Reg](auto Entity, auto &S, auto const &SB) {
        if (SB.SourceCount == 0) {
          Reg.erase<StatsBuffComp>(Entity);
          return;
        }
        S.add(SB.Bonus);
      });
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
        Health.RegenAmount = St.effective().Vit * HealthRegenPerVit;
      });

  // Mana
  // If the entity has stats they override the maximum values defined
  Reg.view<const StatsComp, ManaComp>().each([](const auto &St, auto &Mana) {
    Mana.MaxValue = St.effective().Int * ManaPerInt;
    if (Mana.Value > Mana.MaxValue) {
      Mana.Value = Mana.MaxValue;
    }
    Mana.RegenAmount = St.effective().Int * ManaRegenPerInt;
  });

  // Agility is determined by Dex
  Reg.view<const StatsComp, AgilityComp>().each([](const auto &St, auto &Ag) {
    Ag.Agility = std::log(AgilityPerDexLogBase + St.effective().Dex) /
                 std::log(AgilityPerDexLogBase + AgilityPerDexLogOffset) *
                 AgilityPerDexScale;
  });
}

} // namespace

void StatsSystem::update(UpdateType Type) {
  resetStats(Reg);

  // Update stats buff per hit comp
  applyStatsBuffPerHitComp(Reg, UpdateType::Tick == Type);

  if (Type == UpdateType::Tick) {
    applyTimedStatsAndBuffs(Reg);
  }

  applyStaticStatBuffs(Reg);
  applyStatEffects(Reg);
}

} // namespace rogue
