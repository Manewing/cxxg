#include <cxxg/Screen.h>
#include <memory>
#include <rogue/Components/Buffs.h>
#include <rogue/UI/Controller.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/ItemSelect.h>
#include <rogue/UI/Stats.h>
#include <rogue/UI/Tooltip.h>

static constexpr cxxg::types::Position DefaultPos = {2, 2};
static constexpr cxxg::types::Size DefaultSize = {26, 6};
constexpr cxxg::types::Size TooltipSize = {40, 10};
constexpr cxxg::types::Position TooltipOffset = {1, 1};

struct StatsInfo {
  cxxg::types::Position Offset = {0, 0};
  const char *Short = "<unimp. Stat>";
  const char *Name = "<unimp. Stat>";
  const char *DefaultValue = "<unimp. Stat>";
  const char *Desc = "<unimp. Stat>";
};
static constexpr std::array<StatsInfo, 4> StatsInfos = {{
    {{0, -1}, "Str", "Strength", "<str>", "Strength of the char"},
    {{0, 0}, "Dex", "Dexterity", "<dex>", "Dexterity of the char"},
    {{12, -1}, "Int", "Intelligence", "<int>", "Intelligence of the char"},
    {{12, 0}, "Vit", "Vitality", "<vit>", "Vitality of the char"},
}};

namespace rogue::ui {

StatsController::StatsController(Controller &Ctrl, StatsComp &Stats,
                                 entt::entity Entity, entt::registry &Reg)
    : BaseRectDecorator(DefaultPos, DefaultSize, nullptr), Ctrl(Ctrl),
      Stats(Stats), Entity(Entity), Reg(Reg) {
  ItSel = std::make_shared<ItemSelect>(Pos);

  // Str, Dex, Int, Vit
  for (const auto &SI : StatsInfos) {
    ItSel->addSelect<LabeledSelect>(SI.Short, SI.DefaultValue, Pos + SI.Offset,
                                    10);
  }

  Comp = std::make_shared<Frame>(ItSel, Pos, DefaultSize, "Stats");
}

bool StatsController::handleInput(int Char) {
  switch (Char) {
  case Controls::Info.Char: {
    auto SelIdx = ItSel->getSelectedIdx();
    const auto &SI = StatsInfos.at(SelIdx);
    Ctrl.addWindow(std::make_shared<Tooltip>(Pos + SI.Offset + TooltipOffset,
                                             TooltipSize, SI.Desc, SI.Name));
  } break;
  default:
    return ItSel->handleInput(Char);
  }
  return true;
}

std::string StatsController::getInteractMsg() const {
  std::vector<KeyOption> Options = {Controls::Navigate, Controls::Info,
                                    Controls::SpendPoint};
  return KeyOption::getInteractMsg(Options);
}

namespace {

std::string getStatsStr(StatPoint Base, StatPoint Bonus) {
  return std::to_string(Base) + "+" + std::to_string(Bonus);
}

} // namespace

void StatsController::draw(cxxg::Screen &Scr) const {

  // Update stat values
  ItSel->getSelect(0).setValue(getStatsStr(Stats.Base.Str, Stats.Bonus.Str));
  ItSel->getSelect(1).setValue(getStatsStr(Stats.Base.Dex, Stats.Bonus.Dex));
  ItSel->getSelect(2).setValue(getStatsStr(Stats.Base.Int, Stats.Bonus.Int));
  ItSel->getSelect(3).setValue(getStatsStr(Stats.Base.Vit, Stats.Bonus.Vit));

  BaseRectDecorator::draw(Scr);

  // Add armor info
  if (const auto *ABC = Reg.try_get<ArmorBuffComp>(Entity)) {
    Scr[Pos.Y + getSize().Y - 3][Pos.X + 2] << ABC->getDescription();
  }

  // FIXME points to spend
  Scr[Pos.Y + getSize().Y - 2][Pos.X + 2] << "Available Points: 0000";
}

} // namespace rogue::ui