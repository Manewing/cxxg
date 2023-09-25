#ifndef ROGUE_UI_BUFFS_H
#define ROGUE_UI_BUFFS_H

#include <entt/entt.hpp>
#include <rogue/UI/Decorator.h>

namespace rogue::ui {
class TextBox;
} // namespace rogue::ui

namespace rogue::ui {

class BuffsInfo : public Decorator {
public:
  BuffsInfo() = delete;
  BuffsInfo(entt::entity Entity, entt::registry &Reg);

  void draw(cxxg::Screen &Scr) const final;

private:
  entt::entity Entity;
  entt::registry &Reg;
  std::shared_ptr<TextBox> TB;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_BUFFS_H