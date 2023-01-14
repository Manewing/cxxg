#include <rogue/Components/Buffs.h>
#include <rogue/Components/Stats.h>
#include <rogue/Systems/StatsSystem.h>

namespace rogue {

void StatsSystem::update() {
  // Reset stats
  Reg.view<StatsComp>().each([](auto &S) { S.reset(); });

  // Apply static stats buff
  Reg.view<StatsComp, const StatsBuffComp>().each(
      [](auto &S, auto const &SB) { S.add(SB.Bonus); });

  // Apply timed stats buff
  Reg.view<StatsComp, StatsTimedBuffComp>().each(
      [this](auto Entity, auto &S, auto &STB) {
        // Post decrement to match count
        if (STB.TicksLeft-- == 0) {
          Reg.erase<StatsBuffComp>(Entity);
          return;
        }
        S.add(STB.Bonus);
      });
}

} // namespace rogue