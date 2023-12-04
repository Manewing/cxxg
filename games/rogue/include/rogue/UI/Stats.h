#ifndef ROGUE_UI_STATS_H
#define ROGUE_UI_STATS_H

#include <entt/entt.hpp>
#include <rogue/Components/Stats.h>
#include <rogue/UI/Decorator.h>

namespace rogue::ui {
class Controller;
class ItemSelect;
} // namespace rogue::ui

namespace rogue::ui {

class StatsController : public BaseRectDecorator {
public:
  StatsController() = delete;
  StatsController(Controller &Ctrl, StatsComp &Stats, entt::entity Entity,
                  entt::registry &Reg);
  bool handleInput(int Char) final;
  std::string getInteractMsg() const final;
  void draw(cxxg::Screen &Scr) const final;

protected:
  Controller &Ctrl;
  StatsComp &Stats;
  entt::entity Entity;
  entt::registry &Reg;
  std::shared_ptr<ItemSelect> ItSel;
};

} // namespace rogue::ui

#endif // #define ROGUE_UI_STATS_H