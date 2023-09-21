#ifndef ROGUE_UI_STATS_H
#define ROGUE_UI_STATS_H

#include <entt/entt.hpp>
#include <rogue/Components/Stats.h>
#include <rogue/UI/Decorator.h>

namespace rogue::ui {
class ItemSelect;
}

namespace rogue::ui {

class StatsController : public BaseRectDecorator {
public:
  StatsController() = delete;
  StatsController(StatsComp &Stats, entt::entity Entity, entt::registry &Reg);
  bool handleInput(int Char) final;
  std::string getInteractMsg() const final;
  void draw(cxxg::Screen &Scr) const final;

protected:
  StatsComp &Stats;
  std::shared_ptr<ItemSelect> ItSel;
  mutable std::shared_ptr<Widget> Tp;
};

} // namespace rogue::ui

#endif // #define ROGUE_UI_STATS_H