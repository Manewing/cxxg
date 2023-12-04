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

  if (const auto *MA = Reg.try_get<MeleeAttackComp>(Entity)) {
    if (MA->PhysDamage > 0) {
      SS << MA->PhysDamage << " melee phys. dmg.\n";
    }
    if (MA->MagicDamage > 0) {
      SS << MA->MagicDamage << " melee magic dmg.\n";
    }
  }
  if (const auto *RA = Reg.try_get<RangedAttackComp>(Entity)) {
    if (RA->PhysDamage > 0) {
      SS << RA->PhysDamage << " ranged phys. dmg.\n";
    }
    if (RA->MagicDamage > 0) {
      SS << RA->MagicDamage << " ranged magic dmg.\n";
    }
  }

  auto Text = SS.str();
  if (Text.empty()) {
    Text = "No buffs active";
  }

  TB->setText(Text);
  Comp->draw(Scr);
}

} // namespace rogue::ui