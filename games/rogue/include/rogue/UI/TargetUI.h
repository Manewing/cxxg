#ifndef ROGUE_TARGET_UI_H
#define ROGUE_TARGET_UI_H

#include <entt/entt.hpp>
#include <rogue/UI/Decorator.h>
#include <rogue/UI/Widget.h>
#include <ymir/Types.hpp>

namespace rogue {
class Controller;
class Level;
} // namespace rogue

namespace rogue::ui {

class TargetInfo : public BaseRectDecorator {
public:
  TargetInfo(Controller &Ctrl, entt::entity TargetEt, entt::registry &Reg);
  bool handleInput(int) override;
  void draw(cxxg::Screen &Scr) const override;

private:
  Controller &Ctrl;
  entt::entity TargetEt;
  entt::registry &Reg;
};

class TargetUI : public Widget {
public:
  TargetUI(Controller &Ctrl, entt::entity SrcEt, ymir::Point2d<int> StartPos,
           Level &Lvl);
  bool handleInput(int) override;
  std::string getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

  void destroyCursor();
  void showInfoForTarget(entt::entity TargetEt, entt::registry &Reg);

private:
  Controller &Ctrl;
  Level &Lvl;
  ymir::Point2d<int> StartPos;
  entt::entity SrcEt = entt::null;
  entt::entity CursorEt = entt::null;
  entt::entity TargetEt = entt::null;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_TARGET_UI_H