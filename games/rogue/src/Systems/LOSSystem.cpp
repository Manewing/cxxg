#include <rogue/Components/Buffs.h>
#include <rogue/Components/LOS.h>
#include <rogue/Components/Transform.h>
#include <rogue/Systems/LOSSystem.h>

namespace rogue {

namespace {

void resetLOSComps(entt::registry &Reg) {
  Reg.view<LineOfSightComp>().each([](auto &LOS) { LOS.reset(); });
  Reg.view<VisibleLOSComp>().each([&Reg](auto Et, auto &VLOS) {
    if (VLOS.Temporary) {
      Reg.erase<VisibleLOSComp>(Et);
    }
  });
}

void applyStaticDebuffs(entt::registry &Reg, bool Tick) {
  Reg.view<LineOfSightComp, BlindedDebuffComp>().each(
      [&Reg, Tick](auto Et, auto &LOS, auto &DB) {
        if (Tick && DB.tick() == TimedBuff::State::Expired) {
          Reg.erase<BlindedDebuffComp>(Et);
          return;
        }
        LOS.LOSRange = LOS.LOSRange * DB.Factor;
      });

  Reg.view<MindVisionBuffComp, PositionComp>().each([&Reg, Tick](auto Et,
                                                                 auto &MVB,
                                                                 auto &PC) {
    if (Tick && MVB.tick() == TimedBuff::State::Expired) {
      Reg.erase<MindVisionBuffComp>(Et);
      return;
    }
    Reg.view<LineOfSightComp, PositionComp>().each(
        [&Reg, Et, &PC, &MVB](auto TEt, auto &, auto &TPC) {
          if (TEt == Et) {
            return;
          }
          if (MVB.Range < static_cast<unsigned>((PC.Pos - TPC.Pos).length())) {
            return;
          }
          if (Reg.any_of<VisibleLOSComp>(TEt)) {
            return;
          }
          Reg.emplace<VisibleLOSComp>(TEt).Temporary = true;
        });
  });
}

} // namespace

void LOSSystem::update(UpdateType Type) {
  // Reset
  resetLOSComps(Reg);

  // Apply static de-buffs
  applyStaticDebuffs(Reg, UpdateType::Tick == Type);
}

} // namespace rogue
