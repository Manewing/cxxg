#include <cxxg/Screen.h>
#include <iomanip>
#include <memory>
#include <rogue/Components/Stats.h>
#include <rogue/UI/Controller.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/ItemSelect.h>
#include <rogue/UI/Stats.h>
#include <rogue/UI/Tooltip.h>

static constexpr cxxg::types::Position DefaultPos = {2, 2};
static constexpr cxxg::types::Size DefaultSize = {32, 12};
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
    {{0, 0}, "Str", "Strength", "<str>", "Strength of the char"},
    {{0, 1}, "Dex", "Dexterity", "<dex>", "Dexterity of the char"},
    {{(DefaultSize.X - 2) / 2, 0},
     "Int",
     "Intelligence",
     "<int>",
     "Intelligence of the char"},
    {{(DefaultSize.X - 2) / 2, 1},
     "Vit",
     "Vitality",
     "<vit>",
     "Vitality of the char"},
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
                                    (DefaultSize.X - 6) / 2);
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

  // Add health info
  if (const auto *HC = Reg.try_get<HealthComp>(Entity)) {
    Scr[Pos.Y + getSize().Y - 6][Pos.X + 2] << "HP: " << std::setprecision(2)
                                            << HC->Value << "/" << HC->MaxValue;
  } else {
    Scr[Pos.Y + getSize().Y - 6][Pos.X + 2] << "HP: ---";
  }

  // Add mana info
  if (const auto *MC = Reg.try_get<ManaComp>(Entity)) {
    Scr[Pos.Y + getSize().Y - 5][Pos.X + 2] << "MP: " << std::setprecision(2)
                                            << MC->Value << "/" << MC->MaxValue;
  } else {
    Scr[Pos.Y + getSize().Y - 5][Pos.X + 2] << "MP: ---";
  }

  // Add agility info
  if (const auto *AC = Reg.try_get<AgilityComp>(Entity)) {
    Scr[Pos.Y + getSize().Y - 4][Pos.X + 2]
        << "Agility: " << std::setprecision(2) << AC->Agility
        << ", AP: " << AC->AP;
  } else {
    Scr[Pos.Y + getSize().Y - 4][Pos.X + 2] << "MP: ---";
  }

  // FIXME points to spend
  Scr[Pos.Y + getSize().Y - 2][Pos.X + 2] << "Available Points: 0000";
}

} // namespace rogue::ui