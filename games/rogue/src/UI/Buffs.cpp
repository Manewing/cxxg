#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Helpers.h>
#include <rogue/UI/Buffs.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/TextBox.h>
#include <sstream>

static constexpr cxxg::types::Position DefaultPos = {2, 2};
static constexpr cxxg::types::Size DefaultSize = {40, 16};

namespace rogue::ui {

BuffsInfo::BuffsInfo(entt::entity Entity, entt::registry &Reg)
    : Decorator(DefaultPos, nullptr), Entity(Entity), Reg(Reg) {
  TB = std::make_shared<TextBox>(Pos, DefaultSize, "<to be filled>");
  Comp = std::make_shared<Frame>(TB, Pos, DefaultSize, "Buffs");
}

void BuffsInfo::draw(cxxg::Screen &Scr) const {
  std::stringstream SS;
  applyForComponents<BuffTypeList>([this, &SS](const auto &Comp) {
    auto *BB = static_cast<BuffBase *>(
        Reg.try_get<std::decay_t<decltype(Comp)>>(Entity));
    if (BB) {
      SS << "-> " << BB->getDescription() << "\n";
    }
  });
  auto *SC = Reg.try_get<StatsComp>(Entity);

  const char *Pred = "-> ";
  if (const auto *MA = Reg.try_get<MeleeAttackComp>(Entity)) {
    const auto EffMA = MA->getEffective(SC);
    if (MA->PhysDamage > 0) {
      SS << Pred << MA->PhysDamage << " melee phys. dmg. (eff. "
         << static_cast<int>(EffMA.PhysDamage) << ")";
      Pred = ", ";
    }
    if (MA->MagicDamage > 0) {
      SS << Pred << MA->MagicDamage << " melee magic dmg. (eff. "
         << static_cast<int>(EffMA.MagicDamage) << ")";
      Pred = ", ";
    }
    if (EffMA.APCost > 0) {
      SS << Pred << EffMA.APCost << "AP";
      Pred = ", ";
    }
    if (EffMA.ManaCost > 0) {
      SS << Pred << EffMA.ManaCost << "MP";
      Pred = ", ";
    }
    SS << "\n";
  }
  Pred = "-> ";
  if (const auto *RA = Reg.try_get<RangedAttackComp>(Entity)) {
    const auto EffRA = RA->getEffective(SC);
    if (RA->PhysDamage > 0) {
      SS << Pred << RA->PhysDamage << " ranged phys. dmg. (eff. "
         << EffRA.PhysDamage << ")";
      Pred = ", ";
    }
    if (RA->MagicDamage > 0) {
      SS << Pred << RA->MagicDamage << " ranged magic dmg. (eff. "
         << EffRA.MagicDamage << ")";
      Pred = ", ";
    }
    if (EffRA.APCost > 0) {
      SS << Pred << EffRA.APCost << "AP";
      Pred = ", ";
    }
    if (EffRA.ManaCost > 0) {
      SS << Pred << EffRA.ManaCost << "MP";
      Pred = ", ";
    }
    SS << "\n";
  }

  auto Text = SS.str();
  if (Text.empty()) {
    Text = "No buffs active";
  }

  TB->setText(Text);
  Comp->draw(Scr);
}

} // namespace rogue::ui