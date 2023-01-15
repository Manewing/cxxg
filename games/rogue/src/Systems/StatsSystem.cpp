#include <rogue/Components/Buffs.h>
#include <rogue/Components/Stats.h>
#include <rogue/Systems/StatsSystem.h>

namespace rogue {

namespace {

static constexpr float HealthPerVit = 9.1f;
static constexpr float ManaPerInt = 9.1f;
static constexpr float AgilityPerDex = 2.73f;

void updateStats(entt::registry &Reg) {
  // Reset stats
  Reg.view<StatsComp>().each([](auto &S) { S.reset(); });

  // Apply static stats buff
  Reg.view<StatsComp, const StatsBuffComp>().each(
      [](auto &S, auto const &SB) { S.add(SB.Bonus); });

  // Apply timed stats buff
  Reg.view<StatsComp, StatsTimedBuffComp>().each(
      [&Reg](auto Entity, auto &S, auto &STB) {
        // Post decrement to match count
        if (STB.TicksLeft-- == 0) {
          Reg.erase<StatsBuffComp>(Entity);
          return;
        }
        S.add(STB.Bonus);
      });
}

void applyStatEffects(entt::registry &Reg) {
  // Health
  // If the entity has stats they override the maximum values defined
  Reg.view<const StatsComp, HealthComp>().each(
      [](const auto &St, auto &Health) {
        Health.MaxValue = St.effective().Vit * HealthPerVit;
      });

  // Mana
  // If the entity has stats they override the maximum values defined
  Reg.view<const StatsComp, ManaComp>().each([](const auto &St, auto &Mana) {
    Mana.MaxValue = St.effective().Int * ManaPerInt;
  });

  // Agility is determined by Dex
  Reg.view<const StatsComp, AgilityComp>().each([](const auto &St, auto &Ag) {
    Ag.Agility = AgilityPerDex * St.effective().Dex;
  });
}

} // namespace

void StatsSystem::update() {
  updateStats(Reg);
  applyStatEffects(Reg);
}

} // namespace rogue