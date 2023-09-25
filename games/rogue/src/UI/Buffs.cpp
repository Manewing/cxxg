#include <rogue/Components/Buffs.h>
#include <rogue/Components/Helpers.h>
#include <rogue/UI/Buffs.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/TextBox.h>
#include <sstream>

static constexpr cxxg::types::Position DefaultPos = {2, 2};
static constexpr cxxg::types::Size DefaultSize = {40, 6};

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

  auto Text = SS.str();
  if (Text.empty()) {
    Text = "No buffs active";
  }

  TB->setText(Text);
  Comp->draw(Scr);
}

} // namespace rogue::ui